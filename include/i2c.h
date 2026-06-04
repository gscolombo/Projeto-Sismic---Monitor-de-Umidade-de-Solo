#ifndef I2C_H
#define I2C_H

#include <msp430.h>
#include <stdint.h>

void setupI2C();
uint8_t i2cSend(uint8_t, uint8_t);
uint8_t i2cReceive(uint8_t, uint8_t *);

#endif
