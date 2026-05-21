#ifndef UART_H
#define UART_H

#include <msp430.h>

void setupUART();

void serialPrint(const char *);
void serialPrintLn(const char *);

#endif
