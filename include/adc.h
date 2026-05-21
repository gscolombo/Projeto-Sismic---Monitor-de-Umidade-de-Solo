#ifndef ADC_H
#define ADC_H

#include <msp430.h>

extern volatile unsigned int analog_read_value;

void setupADC12(unsigned int);
void analogRead(volatile unsigned int *);

#endif
