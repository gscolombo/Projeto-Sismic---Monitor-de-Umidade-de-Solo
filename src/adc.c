#include <adc.h>

volatile unsigned int analog_read_value;

void setupADC12(unsigned int channel) {
  if (channel > 7)
    return;

  // Configura pino P6.x para entrada analógica
  unsigned int pin = 1 << channel;

  P6SEL |= pin;
  P6DIR &= ~pin;

  // Configura o ADC12_A (conversor analógico-digital)
  ADC12CTL0 = ADC12SHT0_2; // 16 ciclos de clock para amostragem

  // Modo de amostragem de pulso | Uso de ACLK para clock de amostragem
  ADC12CTL1 = ADC12SHP | ADC12SSEL_1; // ≈ 16 amostras a cada 0,5ms

  ADC12CTL2 = ADC12RES_2; // Resolução de 12 bits

  // Seleção de canal para resultados de conversão
  ADC12MCTL0 = channel;

  // Configura Timer_A0 para leitura periódica de entrada analógica
  TA0CTL = TASSEL__ACLK | MC__UP | TAIE | TACLR;
  TA0CCTL0 = CCIE;
  TA0CCR0 = 32768 - 1; // Período de 1s
}

void analogRead(volatile unsigned int *result) {
  ADC12IFG &= ~ADC12IFG0;        // Limpa flag de leitura anterior (por garantia)
  ADC12CTL0 |= ADC12ON; // Liga o conversor
  ADC12CTL0 |= ADC12ENC | ADC12SC; // Habilita o conversor | Inicia a conversão

  // Aguarda por amostragem e conversão
  while (!ADC12IFG0)
    ;
  *result = ADC12MEM0;

  ADC12CTL0 &= ~ADC12ON; // Desliga o conversor
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void read() { analogRead(&analog_read_value); }
