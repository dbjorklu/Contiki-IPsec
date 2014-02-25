

#ifndef SHT21_H
#define SHT21_H

void sht21_init(void);
void sht21_off(void);

uint16_t sht21_temp(void);
uint16_t sht21_humidity(void);


/*
 * Relative Humidity in percent (h in 12 bits resolution)
 * RH = -4 + 0.0405*h - 2.8e-6*(h*h)
 *
 * Temperature in Celsius (t in 14 bits resolution at 3 Volts)
 * T = -39.60 + 0.01*t
 */

#endif /* SHT21_H */
