#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

// #include "erbium.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/battery-sensor.h"
#include "dev/radio-sensor.h"
#include "dev/temperature-sensor.h"

#if defined (PLATFORM_HAS_SHT11)
#include "dev/sht11-sensor.h"
#endif

#include "er-coap-13.h"
#include "er-coap-13-engine.h"
#include "er-coap-13-observing.h"


#include "dimlight_tree.h"



#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) //PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) //PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
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


