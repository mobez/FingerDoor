#ifndef SI7021_H_
#define SI7021_H_
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "Adafruit_Si7021.h"

typedef struct 
{
  float temp;
  uint8_t humidi;
  float temp_ds;
}meas_si7021;

extern meas_si7021 meas_si;



void init_temp_si(void);
void measuring(void);

#endif /* SI7021_H_ */
