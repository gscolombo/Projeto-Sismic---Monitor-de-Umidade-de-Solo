#ifndef I2C_H
#define I2C_H

#include <msp430.h>
#include <stdint.h>

/** Configura a USCI_B0 em modo I2C mestre (100 kHz, pinos P3.0/P3.1). */
void setupI2C();

/** Envia um byte para o escravo I2C `addr`. Retorna 1 em sucesso (ACK). */
uint8_t i2cSend(uint8_t, uint8_t);

/** Lê um byte do escravo I2C `addr` para `*buf`. Retorna 1 em sucesso (ACK). */
uint8_t i2cReceive(uint8_t, uint8_t *);

#endif
