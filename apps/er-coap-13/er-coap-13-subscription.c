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

#include "contiki.h"
#include "contiki-net.h"

#include "er-coap-13-subscription.h"
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


MEMB(subscriptions_memb, coap_subscription_t, COAP_MAX_OPEN_SUBSCRIPTIONS);
LIST(subscriptions_list);

coap_subscription_t *
coap_new_subscription(const uint8_t *token, int len, blocking_response_handler callback)
{
  coap_subscription_t *t = memb_alloc(&subscriptions_memb);

  if (t)
  {
    memcpy(t->token, token, len);
    t->callback = callback;
    PRINTF("now subscription %x%x\n", t->token[0], t->token[1]);
    list_add(subscriptions_list, t); /* List itself makes sure same element is not added twice. */
  }
  return t;
}

void
coap_clear_subscription(coap_subscription_t *t)
{
  if (t)
  {
    PRINTF("Freeing subscription %u: %p\n", t->token, t);

    list_remove(subscriptions_list, t);
    memb_free(&subscriptions_memb, t);
  }
}

coap_subscription_t *
coap_get_subscription_by_token(const uint8_t *token, int len)
{
  coap_subscription_t *t = NULL;

  for (t = (coap_subscription_t*)list_head(subscriptions_list); t; t = t->next)
  {
    if (0==memcmp(t->token, token, len)) {
      PRINTF("Found subscription for token %u: %p\n", t->token[0], t);
      return t;
    }
  }
  return NULL;
}
