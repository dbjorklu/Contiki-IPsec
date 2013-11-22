
#include "contiki.h"
#include "jsontree.h"
#include "jsonparse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dev/battery-sensor.h"
#include "dev/radio-sensor.h"
#include "dev/temperature-sensor.h"

#include "er-coap-13.h"
#include "er-coap-13-engine.h"
#include "er-coap-13-observing.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

 struct subscription_
{
  uint16_t port; 
  char path[30];
  char value[5];
  char token[1];
  uip_ipaddr_t ipaddr;
//  side_effect callback;
};

//TODO: 
// get rid of timer
// check what happens if you PUT to nonputtible resource 
// subscription, can we override some json with coap?

struct coap_base {
  struct etimer et;
  coap_packet_t subscribe_request[1]; // This way the packet can be treated as pointer as usual. 
  uint8_t request_pending;
  subscription subscriptions[3]; // the subscriptionarray is index by the observe token  

  uint8_t **put_char_buf;
  uint8_t put_char_buf_overflow[MAX_MESSAGE_WITH_CHUNKS-REST_MAX_CHUNK_SIZE];
  uint16_t put_char_index;
};


void get_ip_str(char *dst, int buf_len, uip_ipaddr_t *ipaddr);
struct jsontree_value *find_json_path(struct jsontree_context *json, const char *path, int uri_len);
static void json_copy_string(struct jsonparse_state *parser, char *string, int len);

/*---------------------------------------------------------------------------*/

static int
rssi_get(struct jsontree_context *js_ctx)
{
  char buf[20];
  snprintf(buf, 20, "%d", 111);
  jsontree_write_atom(js_ctx, buf);
  return 0;
}

struct jsontree_callback rssi_callback =
  JSONTREE_CALLBACK(rssi_get, NULL);

static int
neighbours_get(struct jsontree_context *js_ctx)
{
  char buf[20];
  snprintf(buf, 20, "a mote");
  jsontree_write_string(js_ctx, buf);
  return 0;
}

struct jsontree_callback neighbours_callback =
  JSONTREE_CALLBACK(neighbours_get, NULL);

static int
channel_get(struct jsontree_context *js_ctx)
{
  char buf[20];
  snprintf(buf, 20, "26");
  jsontree_write_string(js_ctx, buf);
  return 0;
}

struct jsontree_callback channel_callback =
  JSONTREE_CALLBACK(channel_get, NULL);

#ifdef BATTERY_OPERATED
static int
battery_get(struct jsontree_context *js_ctx)
{
//  char buf[20];
//  snprintf(buf, 20, "22 proc");
  jsontree_write_int(js_ctx, 22);
  return 0;
}

struct jsontree_callback battery_callback =
  JSONTREE_CALLBACK(battery_get, NULL);
#endif


static int
sub1_get(struct jsontree_context *js_ctx)
{
  printf("sub1_get\n");
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
  
  if (strncmp(path, "host", 4) == 0) {
    char tmp[36];
    get_ip_str(tmp, 36, &subscriptions[0].ipaddr);
    jsontree_write_string(js_ctx, tmp); 
  } else if (strncmp(path, "port", 4) == 0) {
    jsontree_write_int(js_ctx, subscriptions[0].port);
  } else if (strncmp(path, "path", 4) == 0) {
    jsontree_write_string(js_ctx, subscriptions[0].path);
  } else if (strncmp(path, "token", 4) == 0) {
    char tmp[]="0x00"; sprintf(tmp, "%x", subscriptions[0].token[0]);
    jsontree_write_string(js_ctx, tmp);
  } else if(strncmp(path, "value", 4) == 0) {
    jsontree_write_string(js_ctx, subscriptions[0].value);
  } 
  return 0;
}


static int
sub1_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  printf("sub1 set ****\n");
  int type;
  int update = 0;
  
  while ((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      printf("checkin\n");
      if(jsonparse_strcmp_value(parser, "host") == 0) {
        printf("host\n");
        jsonparse_next(parser);
        jsonparse_next(parser);
        jsonparse_get_value_as_uip6(parser, &subscriptions[0].ipaddr);
        update++;
      } else if(jsonparse_strcmp_value(parser, "path") == 0) {
        json_copy_string(parser, subscriptions[0].path, sizeof(subscriptions[0].path));
        update++;
      } 
    } else if(jsonparse_strcmp_value(parser, "token") == 0) {
      //
    } else if(jsonparse_strcmp_value(parser, "value") == 0) {
      // 
    } else if(jsonparse_strcmp_value(parser, "port") == 0) {
        jsonparse_next(parser);
        jsonparse_next(parser);
        subscriptions[0].port = jsonparse_get_value_as_int(parser);
        /*if (subscriptions[0].port == 0) {
          callback_port = 0;
          }*/
        update++;
    }
  }
  if (update) // if something changed, a new obsere request shall be sent
    send_observe_request(1);
  return 0;
}
 
struct jsontree_callback sub1_callback =  JSONTREE_CALLBACK(sub1_get, sub1_set);
static struct jsontree_string desc = JSONTREE_STRING("LedLight");
static struct jsontree_string energy_unit = JSONTREE_STRING("Joule");
static struct jsontree_string dimmer_unit = JSONTREE_STRING("[0-15]");


JSONTREE_OBJECT(radio_tree,
                JSONTREE_PAIR("rssi", &rssi_callback),
                JSONTREE_PAIR("neighbours", &neighbours_callback),
                JSONTREE_PAIR("channel", &channel_callback));
                
#ifdef BATTERY_OPERATED
JSONTREE_OBJECT(system_tree,
                JSONTREE_PAIR("radio", &radio_tree),
                JSONTREE_PAIR("battery", &battery_callback));
#else
JSONTREE_OBJECT(system_tree,
                JSONTREE_PAIR("radio", &radio_tree));
#endif

JSONTREE_OBJECT_EXT(sub1_tree,
                    JSONTREE_PAIR("host", &sub1_callback),
                    JSONTREE_PAIR("port", &sub1_callback),
                    JSONTREE_PAIR("path", &sub1_callback),
                    JSONTREE_PAIR("token", &sub1_callback),
                    JSONTREE_PAIR("value", &sub1_callback));

JSONTREE_OBJECT(subscriptions_tree,
                JSONTREE_PAIR("1", &sub1_tree));

/* complete node tree */
JSONTREE_OBJECT(tree,
                JSONTREE_PAIR("system", &system_tree),
                JSONTREE_PAIR("subscriptions", &subscriptions_tree));

/*---------------------------------------------------------------------------*/


static int
putChar(int a) {
  if (put_char_index >= MAX_MESSAGE_WITH_CHUNKS) {
    PRINTF("Too big a resource representation, regardless of blockwise\n");
  } else if (put_char_index >= REST_MAX_CHUNK_SIZE) {
    // the first REST_MAX_CHUNK_SIZE bytes are written to the coap buffer, 
    // then we write to "overflow" buffer which is handled by chunks
    put_char_buf_overflow[put_char_index++ - REST_MAX_CHUNK_SIZE ] = a;
  }
  else {
    // so far everything fits into one coap message
    (*put_char_buf)[put_char_index++] = a;
  }
  return 0;
}

/* set the tree context to point to the location the path indicates  */
/* return -1 if path not found*/
int get_path(struct jsontree_context *json, const char* path, int len) {
  
  struct jsontree_value *v;
  json->values[0] = v = (struct jsontree_value *)&tree;
  jsontree_reset(json);
  
  if (path==0 || len==0) {
    printf("get the root\n");
    // full tree
  } else {
    v = find_json_path(json, path, len);
  } 
  if (v != NULL) {
    json->path = json->depth;
    return 0;
  } else
    return -1;
}

void GET(struct jsontree_context *json) {
  while(jsontree_print_next(json) && json->path <= json->depth) {
  }
}

void PUT(struct jsontree_context *json, const uint8_t* buf) {
  struct jsontree_value *v;
  struct jsontree_callback *c = NULL;

  v = json->values[json->depth];

  if (v->type == JSON_TYPE_CALLBACK) {
    printf("LEaf\n");
    // TODO: Now that we now this is a leaf, we could tell the callbacks about it
    // so they wouldn't need to figure it out by themselves. We could then also
    // be able to use a top-level callback to handle also atomic siblings
    // we are looking at a leaf, and it has a callback, call it, done
    c = (struct jsontree_callback *)v;
    if (c->set != NULL) {
      struct jsonparse_state js;
      jsonparse_setup(&js, (char*)buf, strlen((char*)buf));
      c->set(json, &js);
    }
  } else {
    // this was not a leaf, we have a subtree beneath us, with one ore more callbacks
    while ((v = jsontree_find_next(json, JSON_TYPE_CALLBACK)) != NULL) {
      c = (struct jsontree_callback *)v;
      if (c->set != NULL) {
        struct jsonparse_state js;
        printf("the buf %s\n", buf);
        jsonparse_setup(&js, (char*)buf, strlen((char*)buf));
        c->set(json, &js);
         break; // I believe we never need to run more than one callback
      }
    } 
  }
}


void init(struct jsontree_context *json) {
  static short initialized = 0;
  if (!initialized) {
    json->values[0] = (struct jsontree_value*)&tree;
    json->putchar = putChar;
    
    jsontree_setup(json, (struct jsontree_value*)&tree, putChar);
    jsontree_reset(json);
    json->putchar = putChar;
    
    uip_ip6addr(&(subscriptions[0].ipaddr), 0xaaaa, 0, 0, 0, 0xc30c, 0x0000, 0x0000, 0x0003); /* cooja2 */
    subscriptions[0].port = COAP_DEFAULT_PORT;
    strcpy(subscriptions[0].path, "-");
    strcpy(subscriptions[0].value, "-");
    subscriptions[0].token[0] = 1;
    initialized = 1;
  }
}


struct jsontree_value *
find_json_path(struct jsontree_context *json, const char *path, int uri_len)
{
  struct jsontree_value *v;
  const char *start;
  const char *end;
  int len;

  v = json->values[0];
  start = path;
  do {
    end = strchr(start, '/');
    if(end == start) {
      break;
    }
    if(end != NULL) {
      len = end - start;
      end++;
    } else {
      len = strlen(start);
    }
    if(v->type != JSON_TYPE_OBJECT) {
      v = NULL;
    } else {
      struct jsontree_object *o;
      int i;

      o = (struct jsontree_object *)v;
      v = NULL;
      for(i = 0; i < o->count; i++) {
        if(strncmp(start, o->pairs[i].name, len) == 0) {
          v = o->pairs[i].value;
          json->index[json->depth] = i;
          json->depth++;
          json->values[json->depth] = v;
          json->index[json->depth] = 0;
          break;
        }
      }
    }
    start = end;
  } while(end != NULL && *end != '\0' && v != NULL);
  json->callback_state = 0;
  return v;
}
/*---------------------------------------------------------------------------*/
static void
json_copy_string(struct jsonparse_state *parser, char *string, int len)
{
  jsonparse_next(parser);
  jsonparse_next(parser);
  jsonparse_copy_value(parser, string, len);
}
/*---------------------------------------------------------------------------*/


void get_ip_str(char *dst, int buf_size, uip_ipaddr_t *ipaddr) {
  snprintf(dst, buf_size, 
           "%x:%x:%x:%x:%x:%x:%x:%x", 
           ((uint16_t *)ipaddr)[0], 
           ((uint16_t *)ipaddr)[1], 
           ((uint16_t *)ipaddr)[2], 
           ((uint16_t *)ipaddr)[3], 
           ((uint16_t *)ipaddr)[4], 
           ((uint16_t *)ipaddr)[5], 
           ((uint16_t *)ipaddr)[6], 
           ((uint16_t *)ipaddr)[7]);
}



RESOURCE(root, METHOD_GET | METHOD_PUT | HAS_SUB_RESOURCES, "", "title=\"mote\";rt=\"mote\"");
void
root_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  char *uri_path;
  int len = REST.get_url(request, &uri_path);
  static int32_t my_offset = 0;
  if (len>0)
    uri_path[len] = '\0';

  printf("root_handler preferredsize=%d, offset=%ld my_offset=%ld,maxchunk=%d pathlen=%d\n", preferred_size, *offset, my_offset, REST_MAX_CHUNK_SIZE, len);
  static struct jsontree_context json;
  init(&json);

  if (get_path(&json, uri_path, len) == 0) {

    // Check the offset for boundaries of the resource data. 
    if (*offset>=MAX_MESSAGE_WITH_CHUNKS)
    {
      REST.set_response_status(response, REST.status.BAD_OPTION);
      const char *error_msg = "BlockOutOfScope";
      REST.set_response_payload(response, error_msg, strlen(error_msg));
      return;
    }
  
    if (*offset > 0) {
      int bytes_left;
      // this is a hack (due to bug in coap?). The preferred_size can change, and coap then also changes the offset to something different from what we set it to, which will lead to errors
      if (*offset < REST_MAX_CHUNK_SIZE)
        bytes_left = put_char_index - REST_MAX_CHUNK_SIZE;
      else
        bytes_left = put_char_index - *offset;

      memcpy(buffer, put_char_buf_overflow + *offset-preferred_size, MIN(bytes_left, preferred_size));
      printf("blockwise bytes_left=%d\n", bytes_left);
      if (bytes_left < preferred_size)
        *offset = -1;
      else {
        *offset += preferred_size;
        my_offset += preferred_size;
        printf("setting offset %ld\n", *offset);
      }
      REST.set_response_payload(response, buffer, MIN(bytes_left, preferred_size));
      printf("should stop now\n");
    } else {
      if (REST.get_method_type(request)==METHOD_GET) {
        put_char_index = 0;
        // ** GET **
        put_char_buf = &buffer;
        GET(&json);
        printf("put_char_index %d\n", put_char_index);
        if (put_char_index >= preferred_size) {
          *offset += preferred_size;
          printf("setting offset %ld\n", *offset);
        }
        
        if (buffer[0]=='{')
          REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
        else
          REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      } else {
        // ** PUT **
        const uint8_t *payload = NULL;
        REST.get_request_payload(request, &payload);
        PUT(&json, payload);
      }
      REST.set_response_payload(response, buffer, MIN(REST_MAX_CHUNK_SIZE, put_char_index));
    // REST.set_response_payload(response, buffer, MIN(REST_MAX_CHUNK_SIZE, preferred_size));
    } 
  } else {
    printf("********* !!\n");
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
} 

// The following are just to get the subresources listed in well-known core
RESOURCE(sub1_host, METHOD_GET , "subscriptions/1/host", "title=\"path\";rt=\"uri\"");
void sub1_host_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(sub1_port, METHOD_GET , "subscriptions/1/port", "title=\"port\";rt=\"port\"");
void sub1_port_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(sub1_path, METHOD_GET , "subscriptions/1/path", "title=\"path\";rt=\"uri\"");
void sub1_path_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(sub1_token, METHOD_GET , "subscriptions/1/token", "title=\"token\";rt=\"uri\"");
void sub1_token_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(sub1_value, METHOD_GET , "subscriptions/1/value", "title=\"value\";rt=\"uri\"");
void sub1_value_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}

RESOURCE(system_rssi, METHOD_GET , "system/radio/rssi", "title=\"value\";rt=\"uri\"");
void system_rssi_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(system_neighbours, METHOD_GET , "system/radio/neighbours", "title=\"value\";rt=\"uri\"");
void system_neighbours_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
RESOURCE(system_channel, METHOD_GET , "system/radio/channel", "title=\"channel\";rt=\"uri\"");
void system_channel_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
#ifdef BATTERY_OPERATED
RESOURCE(system_battery, METHOD_GET , "system/battery", "title=\"batery\";rt=\"uri\"");
void system_channel_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {}
#endif

void execActions() {
  int i;
  for (i=0; i < NUM_ACTUATORS; i++) {
    printf("exec action? %d\n", actions[i].actuator_action != NULL);
    if (actions[i].actuator_action != NULL) {
      // TODO: parse the expression
      int value = atoi(subscriptions[0].value) * 15;
      actions[i].actuator_action(value);
    }
  }
}

void send_observe_request(int token) {
  printf("send observer request token=%d path=%s\n", token, subscriptions[token-1].path);
  uint8_t token_[TOKEN_LEN] = {token}; 
// prepare request, TID is set by COAP_BLOCKING_REQUEST() 
  request_pending = token;
  coap_init_message(subscribe_request, COAP_TYPE_CON, COAP_GET, 0 );
  coap_set_header_uri_path(subscribe_request, subscriptions[token-1].path);
  coap_set_header_observe(subscribe_request, 0xabcd);
  coap_set_header_token(subscribe_request, token_, TOKEN_LEN);
  // TODO: process_poll(dimlight);
}

// This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. 
void
notification_handler(void *response)
{
  printf("notified\n");
  const uint8_t *chunk;
  int len = coap_get_payload(response, &chunk);
  
  uint8_t token = ((coap_packet_t*)response)->token[0]; 
  if (len)
  {
    strncpy(subscriptions[token-1].value, (const char*)chunk, 4);
  }
  // TODO: now just running all actions, should run actions depending on this vlaue

  execActions(atoi(subscriptions[token-1].value));
}

void coap_base_init() {
  etimer_set(&et, 1 * CLOCK_SECOND);
  // Initialize the REST engine. 
  rest_init_engine();
  // receives all CoAP messages
  coap_receiver_init();
  rest_activate_resource(&resource_root);
  
  
  // The following are subresources to subscription1. As the are activated after the top resource, the handles for these will actually never be called 
  rest_activate_resource(&resource_sub1_host);
  rest_activate_resource(&resource_sub1_port);
  rest_activate_resource(&resource_sub1_path);
  rest_activate_resource(&resource_sub1_token);
  rest_activate_resource(&resource_sub1_value);

  rest_activate_resource(&resource_system_rssi);
  rest_activate_resource(&resource_system_neighbours);
  rest_activate_resource(&resource_system_channel);
  rest_activate_resource(&resource_system_battery);
}



