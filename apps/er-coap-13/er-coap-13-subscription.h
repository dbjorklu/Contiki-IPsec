/*
 * Copyright (c) 2013, Dag Bjorklund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      CoAP module for handling client subscriptions to observable resources
 * \author
 *      Dag Bjorklund <dag.bjorklund@comsel.com>
 */

#ifndef COAP_SUBSCRIPTION_H_
#define COAP_SUBSCRIPTION_H_

#include "er-coap-13.h"
#include "er-coap-13-engine.h"
typedef void (*blocking_response_handler) (void* response);
/*
 * The number of concurrent messages that can be stored for retransmission in the transaction layer.
 */
#ifndef COAP_MAX_OPEN_SUBSCRIPTIONS
#define COAP_MAX_OPEN_SUBSCRIPTIONS 4
#endif /* COAP_MAX_OPEN_SUBSCRIPTIONS */

/* container for transactions with message buffer and retransmission info */
typedef struct coap_subscription {
  struct coap_subscription *next; /* for LIST */
  blocking_response_handler callback;
  uint8_t token[COAP_TOKEN_LEN];
} coap_subscription_t;

coap_subscription_t *coap_new_subscription(const uint8_t *token, int len, blocking_response_handler callback);
void coap_clear_subscription(coap_subscription_t *t);
coap_subscription_t *coap_get_subscription_by_token(const uint8_t *token, int len);


#endif /* COAP_SUBSCRIPTION_H_ */

