#ifndef BUZZER_H
#define BUZZER_H

#include <msp430.h>

// Definição do pino do Buzzer na Porta 1 
#define BUZZER_DIR P1DIR
#define BUZZER_OUT P1OUT
#define BUZZER_PIN BIT5

void setupBuzzer(void);
void buzzerOn(void);
void buzzerOff(void);

#endif