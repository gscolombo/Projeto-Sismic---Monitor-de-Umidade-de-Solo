#include <adc.h>

volatile unsigned int value;

void setupADC12() {
  // Configura pino P6.0 para entrada analógica
  P6SEL |= BIT0;

  // Configura pino P6.5 para alimentação do sensor
  P6DIR |= BIT5;
  P6OUT &= ~BIT5;

  ADC12CTL0 &= ~ADC12ENC;

  ADC12CTL0 |=
      ADC12ON | ADC12SHT0_4; // Liga o ADC | 64 ciclos de ADC12CLK (SMCLK)

  ADC12CTL1 = ADC12SSEL_2     // SMCLK
              | ADC12SHP      // Amostragem por pulso
              | ADC12CONSEQ_2 // Modo repeat-single-channel
              | ADC12SHS_1;   // Trigger por TimerA0.1

  ADC12CTL2 = ADC12RES_2; // Resolução de 12 bits

  // Seleção de canal para resultados de conversão
  ADC12MCTL0 = ADC12INCH_0;
  ADC12IE = ADC12IE0; // Habilita interrupção no canal 0

  // Configura Timer_A0 para conversão periódica
  TA0CTL = TASSEL__ACLK | MC__UP | TAIE | TACLR;
  TA0CCTL1 = OUTMOD_2; // Toggle/Reset
  TA0CCR0 = 32768 - 1; // Período de 1s
  TA0CCR1 = 3277 - 1;

  ADC12CTL0 |= ADC12ENC; // Habilita a conversão
}

#pragma vector = ADC12_VECTOR
__interrupt void read() {
  switch (ADC12IV) {
  case ADC12IV_ADC12IFG0:
    value = ADC12MEM0;
    P6OUT &= ~BIT5;
    break;
  default:
    break;
  }
}
