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
 *         Sensor driver for reading SHT21 sensor, humidity and temperature
 * \author
 *         Dag Björklund, Johan Westö
 */

#include "contiki.h"
#include "dev/sht21.h"
#include "dev/sht21-sensor.h"
#include <util/twi.h>


#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

const struct sensors_sensor sht21_sensor;


enum twi_err {START_FAILED, SET_ADDR_FAILED, POLL_FAILED, 
              WRITE_COMMAND_FAILED, WRITE_VAL_FAILED, READ_ACK_FAILED };
static enum twi_err error;


/*---------------------------------------------------------------------------*/
static void TWIInit(void)
{
  PRINTF("TWIInit\n");
  //set SCL to 400kHz
  TWSR = 0x00;
  TWBR = 0x02;  
  //TWBR = (F_CPU / 100000UL -16) / 2;
  //PRINTF("df %d\n", (F_CPU / 100000UL -16) / 2);
  //  SCL frequncy = CPU Clock frequency / (16+2*TWBR*4^TWPS)
  //enable TWI
  TWCR = (1<<TWEN);
}

uint8_t writeUserRegister(uint8_t val);
uint16_t read2ByteRegister(uint8_t address, uint8_t command);
uint8_t readRegisterByte1(uint8_t address, uint8_t command);
uint8_t readRegisterByte2AndCRC();

void sht21_init() 
{
  // using internal pullups
  DDRD &= ~0x3; // make input
  PORTD |= 0x3; // enable pull

  TWIInit();
  // use lower resolution to limit acquisition time
  // uint8_t writeReg = 0b10000001;
  //writeUserRegister(writeReg);
  PRINTF("sht21_init done\n");
}
 
// Send start signal
static void TWIStart(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while ((TWCR & (1<<TWINT)) == 0) {
    PRINTF(".");
  }
}

//send stop signal
static void TWIStop(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

static void TWIWrite(uint8_t u8data)
{
  PRINTF("twi write\n");
  TWDR = u8data;
  TWCR = (1<<TWINT)|(1<<TWEN);
  while ((TWCR & (1<<TWINT)) == 0);
}

static uint8_t TWIReadACK(void)
{
  PRINTF("twi read ack\n");
  TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
  while ((TWCR & (1<<TWINT)) == 0);
  PRINTF("twi got ack\n");
  return TWDR;
}

static uint8_t TWIGetStatus(void)
{
  uint8_t status;
  //mask status
  status = TWSR & 0xF8;
  return status;
}



uint16_t sht21_temp() {
  return read2ByteRegister(0b1000000, 0b11100011);
}

uint16_t sht21_humidity() {
  return read2ByteRegister(0b1000000, 0b11100101);
}

uint8_t readUserRegister() {
  return readRegisterByte1(0b1000000, 0b11100111);
}

// Read 16b register
uint16_t read2ByteRegister(uint8_t address, uint8_t command) {
  uint8_t byte;
  uint16_t result;
  byte = readRegisterByte1(address, command);
  if (byte) {
    result = byte;
    result = result << 8;
    byte = readRegisterByte2AndCRC();
    if (byte) 
      result += byte;
    else
      result = 0;
    return result;
  } else {
    PRINTF("Error reading first byte %d\n", error);
    return result;
  }
}

// Read first byte of 16b register, or an 8b register
uint8_t readRegisterByte1(uint8_t address, uint8_t command) {
  uint8_t temp;
  PRINTF("readRegisterByte1\n");
  TWIStart();
  
  if ((TWIGetStatus() != TW_START) && (TWIGetStatus() != TW_REP_START)) {
    error = START_FAILED;
    return 0;
  }

  // prepare for write, send address
  TWIWrite( (address<<1) | TW_WRITE);
  if ((TWIGetStatus() !=  TW_MT_SLA_ACK) && (TWIGetStatus() != TW_MT_SLA_NACK)) {
    error = SET_ADDR_FAILED;
    return 0;
  }

  // write command
  TWIWrite(command);
  if ((TWIGetStatus() != TW_MT_DATA_ACK) && (TWIGetStatus() != TW_MT_DATA_NACK)) {
    error = WRITE_COMMAND_FAILED;
    return 0;
  }
  
  // polling for result
  TWIStart();
  if ((TWIGetStatus() != TW_START) && (TWIGetStatus() != TW_REP_START)) {
    error = POLL_FAILED;
    return 0;
  }

  // send adress
  TWIWrite((address<<1) | TW_READ);
  if ((TWIGetStatus() != TW_MR_SLA_ACK) & (TWIGetStatus() != TW_MR_SLA_NACK)) {
    error = SET_ADDR_FAILED;
    return 0;
  }
  // read first byte
  temp = TWIReadACK();
  if (TWIGetStatus() != TW_MR_DATA_ACK) {
    error = READ_ACK_FAILED;
    return 0;
  }
  return temp;
}

// Read the second byte of a 2 Byte register
uint8_t readRegisterByte2AndCRC() {
  uint8_t val, crc;
  val = TWIReadACK();
  if (TWIGetStatus() != TW_MR_DATA_ACK) {
    error = READ_ACK_FAILED;
    return 0;
  }
  
  // read checksum
  crc = TWIReadACK();
  if (TWIGetStatus() != TW_MR_DATA_ACK) {
    error = READ_ACK_FAILED;
    return 0;
  }
  TWIStop();
  // TODO: check CRC

  return val;
}


uint8_t writeUserRegister(uint8_t val) {
  uint8_t adress = 0b1000000;
  uint8_t writeUserReg = 0b11100110;
  PRINTF("writeUserRegister\n");
  // Writing to user register
  TWIStart();
  if ((TWIGetStatus() != TW_START) & (TWIGetStatus() != TW_REP_START)) {
    error = START_FAILED;
    return 0;
  }
  // send adress
  TWIWrite((adress<<1) | TW_WRITE);
  if ((TWIGetStatus() != TW_MT_SLA_ACK) & (TWIGetStatus() != TW_MT_SLA_NACK)) {
    error = SET_ADDR_FAILED;
    return 0;
  }
  // write command
  TWIWrite(writeUserReg);
  if ((TWIGetStatus() != TW_MT_DATA_ACK) & (TWIGetStatus() != TW_MT_DATA_NACK)) {
    error =  WRITE_COMMAND_FAILED;
    return 0;
  }
  // write value
  TWIWrite(val);
  if ((TWIGetStatus() != TW_MT_DATA_ACK) & (TWIGetStatus() != TW_MT_DATA_NACK)) {
    error =  WRITE_VAL_FAILED;
    return 0;
  }
  TWIStop();
  return 1;
}

