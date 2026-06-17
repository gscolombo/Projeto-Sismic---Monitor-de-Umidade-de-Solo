#ifndef SENSOR_H
#define SENSOR_H

#include <adc.h>
#include <lcd.h>

#include <stdio.h>
#include <string.h>

#define DEN 6554
#define WET 2500
#define DRY 4080

typedef struct Percent {
  unsigned int i;
  unsigned int d;
} Percent;

Percent convertRead();

void checkThreshold(Percent *, unsigned int);

#endif
