#ifndef UART_H
#define UART_H

#include <msp430.h>

/** Configura a USCI_A1 em modo UART, 9600 bps (pinos P4.4 TX / P4.5 RX). */
void setupUART();

/** Envia uma string pela UART (bloqueante). */
void serialPrint(const char *);

/** Envia uma string seguida de \r\n pela UART (bloqueante). */
void serialPrintLn(const char *);

#endif
