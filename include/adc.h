#ifndef ADC_H
#define ADC_H

#include <msp430.h>

/** Última leitura bruta do ADC12 (canal 0 / P6.0), atualizada pela ISR. */
extern volatile unsigned int value;

/** Configura o ADC12 (canal 0), a alimentação do sensor e o Timer_A0 de trigger. */
void setupADC12();

#endif
