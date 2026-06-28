/**
 * adc.c
 *
 * Configuração do ADC12 (canal 0, P6.0) e do Timer_A0 que dispara
 * conversões periódicas, além do controle de alimentação do sensor
 * (P6.5) para mitigar efeitos de eletrólise no elemento resistivo.
 */

#include <adc.h>

/**
 * Última leitura bruta do ADC12 (canal 0 / P6.0), em [0, 4095] (12
 * bits). Atualizada pela ISR do ADC12 e consumida por convertRead()
 * em sensor.c. Declarada `volatile` pois é compartilhada entre o
 * contexto de interrupção (escrita) e o contexto principal (leitura).
 */
volatile unsigned int value;

/**
 * Configura o canal 0 do ADC12 (pino P6.0) em modo de amostragem
 * repetida (repeat-single-channel), disparada periodicamente pelo
 * Timer_A0, além do pino de alimentação do sensor (P6.5) e do próprio
 * Timer_A0:
 *   - P6.5 liga/desliga a alimentação do sensor antes e depois de
 *     cada conversão (reduz efeitos de eletrólise no sensor
 *     resistivo, evitando mantê-lo energizado continuamente);
 *   - o ADC12 amostra com 64 ciclos de ADC12CLK (SHT0_4), clock
 *     SMCLK, trigger por TimerA0.1 (SHS_1) e resolução de 12 bits;
 *   - o Timer_A0 gera o trigger de conversão a cada ~1 s (TACCR0 =
 *     32768-1 com ACLK) e também alimenta a ISR de overflow usada em
 *     fsm.c para acordar a FSM principal.
 */
void setupADC12() {
  // Configura pino P6.0 para entrada analógica
  P6SEL |= BIT0;

  // Configura pino P6.5 para alimentação do sensor
  P6DIR |= BIT5;
  P6OUT &= ~BIT5; // sensor inicialmente desligado

  ADC12CTL0 &= ~ADC12ENC; // garante o ADC desabilitado antes de configurar

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
  TA0CCR1 = 3277 - 1;  // ~100ms após o início do período: dispara a conversão

  ADC12CTL0 |= ADC12ENC; // Habilita a conversão
}

#pragma vector = ADC12_VECTOR
/**
 * ISR do ADC12, disparada ao fim de cada conversão do canal 0.
 * Armazena o resultado em `value` e desliga a alimentação do sensor
 * (P6.5) imediatamente após a leitura, minimizando o tempo em que o
 * sensor permanece energizado.
 */
__interrupt void read() {
  switch (ADC12IV) {
  case ADC12IV_ADC12IFG0:
    value = ADC12MEM0;
    P6OUT &= ~BIT5; // desliga a alimentação do sensor após a leitura
    break;
  default:
    break;
  }
}
