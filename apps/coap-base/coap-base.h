#ifndef DIMLIGHT_TREE_
#define DIMLIGHT_TREE_

// typedef void (*side_effect) (void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"
#include "er-coap-13.h"
#include "jsontree.h"

#define MAX_MESSAGE_WITH_CHUNKS   200

typedef void (*action) (int value);



struct ActuatorExpression {
  char *expression;
  action actuator_action;
};

struct subscription_
{
  uint16_t port; 
  char path[30];
  char value[5];
  char token[1];
  uip_ipaddr_t ipaddr;
//  side_effect callback;
};



int get_path(struct jsontree_context *json, const char* path, int len);
typedef struct subscription_ subscription_t;


//extern uint8_t request_pending;
//extern coap_packet_t send_request[1]; /* This way the packet can be treated as pointer as usual. */


#define NUM_ACTUATORS 1

void execActions();

void coap_base_init();
void GET(struct jsontree_context *json);
void PUT(struct jsontree_context *json, const uint8_t* buf);

#define TOKEN_LEN 1

#endif
