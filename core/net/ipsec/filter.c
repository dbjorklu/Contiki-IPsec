#include <contiki.h>
#include "ipsec.h"
#include "sad.h"
#include "spd.h"
#include "common_ipsec.h"

/**
  * Filters incoming traffic in accordance with RFC 4301, section 5.2.
  *
  * \return 0 implies that the packet should be let through, 1 that it should be dropped.
  */
u8_t ipsec_filter(sad_entry_t *sad_entry, ipsec_addr_t *addr)
{
  #define PRINTF printf
//#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
  /*PRINTF("Packet with address:\n");
  PRINTADDR(addr);
  */
  if (sad_entry) {
    /**
      * This packet was protected.
      *
      * Assert that the packet's addr is a subset of the SA's selector
      * (p. 62 part 4 and 5)
      * We don't implement the IKE notification as described in part 5.
      *
      * The reason that we don't assert this earlier is that the next layer
      * might enjoy confidentiality protection and hence we must decrypt it first to
      * get the port numbers from the next layer protocol.
      */
    
    if (ipsec_a_is_member_of_b(addr, &sad_entry->traffic_desc)) {
      // FIX: Update SA statistics    
      return 0;
    }
      
    // Drop the packet
    PRINTF("Dropping packet because the SAD's (referenced by the packet's SPI) selector didn't match the packet\n");
  }
  else {
    /*
     * This packet was unprotected. We fetch the SPD entry so that we can verify that this is in accordance
     * with our policy.
     */
    spd_entry_t *spd_entry = spd_get_entry_by_addr(addr);
    
    switch (spd_entry->proc_action) {
      case SPD_ACTION_BYPASS:
      return 0;
    
      case SPD_ACTION_PROTECT:
      /**
        * Unprotected packets that match a PROTECT policy MUST 
        *   1) be discarded 
        *   2) there should not be any attempt of negotiating an SA
        * (3b. p. 62)
        */
      PRINTF("Dropping unprotected packet (policy PROTECT)\n");
      break;
      
      case SPD_ACTION_DISCARD:
      PRINTF("Dropping unprotected packet (policy DISCARD)\n");
    }
  }
      #define PRINTF
    #define PRINT6ADDR
  return 1; // Drop the packet 
}