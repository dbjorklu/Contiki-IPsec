/**
 * @file
 * @brief This file contains RF230-formatted register definitions for the atmega128rfa1
 */
/*   Copyright (c) 2008, Swedish Institute of Computer Science

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PHY2564RFR2_REGISTERMAP_EXTERNAL_H
#define PHY2564RFR2_REGISTERMAP_EXTERNAL_H

/* RF230 register access is through SPI which transfers 8 bits address/8 bits data.
 * ATmega2564rfr2 registers are defined in I/O space, e.g. in gcc /include/avr/iom2564rfr2.h
 * A typical definition is #define TRX_STATUS _SFR_MEM8(0x141)
 * Registers can be read with a macro, but the args for subregisters don't expand properly so the actual address
 * is used with explicit _SFR_MEM8 in the subregister read/write routines.
 */
#define RG_TRX_STATUS         TRX_STATUS
#define SR_TRX_STATUS         0x141, 0x1f, 0
#define SR_TRX_CMD            0x142, 0x1f, 0
#define STATE_TRANSITION      (31)
#define SR_TX_PWR             0x145, 0x0f, 0
#define RG_VERSION_NUM        VERSION_NUM
#define RG_MAN_ID_0           MAN_ID_0
#define RG_IRQ_MASK           IRQ_MASK
#define SR_MAX_FRAME_RETRIES  0x16C, 0xf0, 4
#define SR_TX_AUTO_CRC_ON     0x144, 0x20, 5
#define SR_TRAC_STATUS        0x142, 0xe0, 5
#define SR_CHANNEL            0x148, 0x1f, 0
#define SR_CCA_MODE           0x148, 0x60, 5
#define SR_CCA_REQUEST        0x148, 0x80, 7
#define RG_PAN_ID_0           PAN_ID_0
#define RG_PAN_ID_1           PAN_ID_1
#define RG_SHORT_ADDR_0       SHORT_ADDR_0
#define RG_SHORT_ADDR_1       SHORT_ADDR_1
#define RG_IEEE_ADDR_0        IEEE_ADDR_0
#define RG_IEEE_ADDR_1        IEEE_ADDR_1
#define RG_IEEE_ADDR_2        IEEE_ADDR_2
#define RG_IEEE_ADDR_3        IEEE_ADDR_3
#define RG_IEEE_ADDR_4        IEEE_ADDR_4
#define RG_IEEE_ADDR_5        IEEE_ADDR_5
#define RG_IEEE_ADDR_6        IEEE_ADDR_6
#define RG_IEEE_ADDR_7        IEEE_ADDR_7
//#define SR_ED_LEVEL           0x147, 0xff, 0
#define RG_PHY_ED_LEVEL       PHY_ED_LEVEL
#define RG_RX_SYN             RX_SYN
#define SR_RSSI               0x146, 0x1f, 0
#define SR_PLL_CF_START       0x15a, 0x80, 7
#define SR_PLL_DCU_START      0x15b, 0x80, 7
#define SR_MAX_CSMA_RETRIES   0x16c, 0x0e, 1
#define RG_CSMA_BE            CSMA_BE
#define RG_CSMA_SEED_0        CSMA_SEED_0
#define RG_PHY_RSSI           PHY_RSSI
//#define SR_CCA_CS_THRES       0x149, 0xf0, 4
#define SR_CCA_ED_THRES        0x149, 0x0f, 0
#define SR_CCA_DONE            0x141, 0x80, 7
#define SR_CCA_STATUS          0x141, 0x40, 6
#define SR_AACK_SET_PD         0x16e, 0x20, 5


/* RF230 register assignments, for reference */
#if 1
/** Constant BUSY_RX for sub-register @ref SR_TRX_STATUS */
#define BUSY_RX                  (1)
/** Constant BUSY_TX for sub-register @ref SR_TRX_STATUS */
#define BUSY_TX                  (2)
/** Constant RX_ON for sub-register @ref SR_TRX_STATUS */
#define RX_ON                    (6)
/** Constant TRX_OFF for sub-register @ref SR_TRX_STATUS */
#define TRX_OFF                  (8)
/** Constant PLL_ON for sub-register @ref SR_TRX_STATUS */
#define PLL_ON                   (9)
/** Constant SLEEP for sub-register @ref SR_TRX_STATUS */
//#define SLEEP                    (15)
/** Constant BUSY_RX_AACK for sub-register @ref SR_TRX_STATUS */
#define BUSY_RX_AACK             (17)
/** Constant BUSY_TX_ARET for sub-register @ref SR_TRX_STATUS */
#define BUSY_TX_ARET             (18)
/** Constant RX_AACK_ON for sub-register @ref SR_TRX_STATUS */
#define RX_AACK_ON               (22)
/** Constant TX_ARET_ON for sub-register @ref SR_TRX_STATUS */
#define TX_ARET_ON               (25)
/** Constant RX_ON_NOCLK for sub-register @ref SR_TRX_STATUS */
#define CMD_FORCE_TRX_OFF        (3)

#endif
#endif /* PHY2564RFR2_REGISTERMAP_EXTERNAL_H */
