/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISE OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Dag Björklund
 * Created : 2005-11-01
 * Updated : $Date: 2010/08/25 19:30:52 $
 *           $Revision: 1.11 $
 */


#include "contiki.h"
#include "dev/battery-sensor.h"

const struct sensors_sensor battery_sensor;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{

  uint16_t h;
  //Measuring AVcc might be useful to check on battery condition but on ext power it's always 3v3
  // Table 27-11 in rfr2 datasheet
  //ADMUX = 0;  // AREF as reference
  ADMUX = (1<<REFS0); // AVDD as reference 
  //ADMUX = (1<<REFS1); // internal 1.5V as reference 
  //ADMUX = (1<<REFS0) | (1<<REFS1); // Internal 1.6V as reference
  
  ADMUX |= 0x1E;    // measure 1.2V bandgap (Vbg) Table 27-12 in rfr2 datasheet
  
  ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); //Enable ADC, interrupt disabled, clock divider  128 (62 KHz@ 8 MHz)
  ADCSRA|= (1<<ADSC);          //Start throwaway conversion
  while (ADCSRA&(1<<ADSC)); // Wait till done
  ADCSRA|=1<<ADSC;          //Start another conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  ADCSRA = 0; // disable ADC
  //Get supply voltage, paramaeters obtained by calibrating agains power supply
  return (int)(-3.5001*(float)ADC + 2440.10);
}

/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(battery_sensor, BATTERY_SENSOR, value, configure, status);

