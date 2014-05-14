/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Sensor driver for reading the temperature sensor in the Atmel Xplained Pro Board
 * \author
 *         Dag Björklund <dag@iki.fi>
 */

#include "dev/temperature-sensor.h"
#include "contiki.h"
#include <util/twi.h>

#define INPUT_CHANNEL      (1 << INCH_10)
#define INPUT_REFERENCE    SREF_1
#define TEMPERATURE_MEM    ADC12MEM10

const struct sensors_sensor temperature_sensor;
static int8_t ReadTemp73(void);
static void TWIInit(void);

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return ReadTemp73();
} 
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  TWIInit();
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 0;
}

/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(temperature_sensor, TEMPERATURE_SENSOR,
               value, configure, status);

static void TWIInit(void)
{
    //set SCL to 400kHz
    TWSR = 0x00;
    //TWBR = 0x06;  
    TWBR = (F_CPU / 100000UL -16) / 2;
    //  SCL frequncy = CPU Clock frequency / (16+2*TWBR*4^TWPS)
    //enable TWI
    TWCR = (1<<TWEN);
}

// Send start signal
static void TWIStart(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//send stop signal
static void TWIStop(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

static void TWIWrite(uint8_t u8data)
{
  TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

static uint8_t TWIReadACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

static uint8_t TWIGetStatus(void)
{
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

 
int8_t ReadTemp73(void)
{
  //  uint8_t adressRead = b10010011;
  uint8_t adressRead = 0b1001011; //   0b10010001; 
  uint8_t temp;
  uint16_t tempData = 0;
  TWIInit();

  TWIStart();
  
  if (TWIGetStatus() != TW_START)
    return 1;

  //select devise and send A2 A1 A0 address bits
  TWIWrite( (adressRead<<1) | TW_READ );

  if (TWIGetStatus() != TW_MR_SLA_ACK)
    return 2;

  // read first byte
  temp = TWIReadACK();

  if (TWIGetStatus() != TW_MR_DATA_ACK)
    return 3;

  tempData += temp;
  tempData = tempData << 8;
  // read second byte
  temp = TWIReadACK();
  if (TWIGetStatus() != TW_MR_DATA_ACK)
    return 4;
  
  tempData += temp;  
  
  TWIStop();

  // 128 - (2^11-t>>4)*0.0625
  return (int)(128 - (2048-(tempData>>4))*0.0625);
}
