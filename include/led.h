#ifndef LED_H
#define LED_H

#include <msp430.h>

// Definição dos pinos do LED RGB na Porta 1
#define LED_DIR P1DIR
#define LED_OUT P1OUT
#define LED_RED_PIN BIT6
#define LED_GREEN_PIN BIT3
#define LED_BLUE_PIN BIT4

// Enumeração para facilitar a legibilidade dos estados do LED
typedef enum { 
    LED_OFF, 
    LED_RED, 
    LED_GREEN, 
    LED_BLUE 
} LedColor;

void setupLED(void);
void setLEDColor(LedColor color);

#endif
