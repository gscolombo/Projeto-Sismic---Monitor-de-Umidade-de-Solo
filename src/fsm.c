/**
 * fsm.c
 *
 * Estado global da máquina de estados principal, configuração dos
 * botões S1/S2 e suas respectivas ISRs (incluindo debounce por
 * software via Timer_A1), e a ISR de overflow do Timer_A0 que
 * sincroniza a FSM com o ciclo periódico de leitura do ADC.
 */

#include <fsm.h>
#include <buzzer.h>

/**
 * Estado atual e estado anterior da máquina de estados principal (ver
 * main.c). `prev_state` é usado para detectar transições e desenhar o
 * conteúdo estático do LCD apenas uma vez por entrada em cada estado.
 */
State state = START, prev_state = START;

/** Limiar de alarme de umidade (%), ajustável pelo usuário em CONFIG via S2. */
uint16_t threshold = 50;

/**
 * Flags de "botão pressionado": usados para ignorar novas
 * interrupções de S1/S2 enquanto o debounce daquele botão ainda está
 * em andamento.
 */
uint8_t s1Pressed = 0, s2Pressed = 0;

/**
 * Configura os botões S1 (P2.1) e S2 (P1.1) como entradas com
 * pull-up, com interrupção na borda de subida (acionada ao soltar o
 * botão), e configura o Timer_A1 (modo contínuo) usado para o
 * debounce por software de ambos os botões (CCR1 para S1, CCR2 para
 * S2, 20 ms cada).
 */
void setupSwitches() {
  P2DIR &= ~BIT1; // Entrada
  P2REN |= BIT1;  // Habilitar resistor pull-up/down
  P2OUT |= BIT1;  // Resistor pull-up
  P2IE |= BIT1;   // Habilitar interrupções
  P2IES &= ~BIT1; // Interrupção na borda de subida

  P1DIR &= ~BIT1;
  P1REN |= BIT1;
  P1OUT |= BIT1;
  P1IE |= BIT1;
  P1IES &= ~BIT1;

  // Configura TimerA1 para debounce
  TA1CTL = TASSEL__SMCLK | MC__CONTINUOUS | TACLR;
  TA1CCR1 = 20000 - 1; // 20ms
  TA1CCR2 = 20000 - 1;
}

#pragma vector = TIMER0_A1_VECTOR
/**
 * ISR de overflow do Timer_A0 (TA0IV_TA0IFG), disparada periodicamente
 * (ciclo de conversão do ADC configurado em adc.c). Liga a
 * alimentação do sensor (P6.5) e, se o sistema estiver em START ou
 * IDLE, transiciona para WORK e tira a CPU do modo de baixo consumo
 * para que o loop principal processe a nova leitura.
 */
__interrupt void checkRead() {
  switch (TA0IV) {
  case TA0IV_TA0IFG:
    P6OUT |= BIT5; // Liga a alimentação do sensor antes da conversão

    if (state == START) {
      state = WORK;
      prev_state = START;
      __low_power_mode_off_on_exit();
    }

    if (state == IDLE) {
      state = WORK;

      // Se estávamos em IDLE vindos de CONFIG, as interrupções dos
      // botões foram desabilitadas ao entrar em CONFIG (ver ISR s1);
      // aqui os flags pendentes são limpos e as interrupções
      // reabilitadas para retomar a operação normal.
      if (prev_state == CONFIG) {
        P1IFG &= ~BIT1;
        P2IFG &= ~BIT1;
        P1IE |= BIT1;
        P2IE |= BIT1;
      }
      prev_state = IDLE;
      __low_power_mode_off_on_exit();
    }
    break;
  default:
    break;
  }
}

#pragma vector = PORT2_VECTOR
/**
 * ISR de S1 (P2.1), disparada na borda de subida (ao soltar o botão).
 * S1 alterna o sistema entre o modo de operação normal (WORK/IDLE) e
 * o modo de configuração (CONFIG):
 *   - Em WORK ou IDLE: desliga a interrupção de overflow do Timer_A0 e
 *     a alimentação do sensor (P6.5), e entra em CONFIG.
 *   - Em CONFIG (ou qualquer outro estado, via default): desabilita
 *     temporariamente as interrupções de S1/S2 (serão reabilitadas
 *     pela ISR do Timer_A0 ao retomar a leitura) e volta para IDLE.
 * Em seguida, inicia o debounce por software via Timer_A1/CCR1.
 */
__interrupt void s1() {
  switch (P2IV) {
  case P2IV_P2IFG1:
    if (!s1Pressed) {  // Ignora a interrupção se o debounce já estiver ativo
      TA0CTL &= ~TAIE; // Desabilita interrupção de overflow do TimerA0
      P6OUT &= ~BIT5;  // Desliga ADC

      // Alterna entre estados do sistema
      switch (state) {
      case WORK:
      case IDLE:
        state = CONFIG;
        __low_power_mode_off_on_exit();
        break;
      case CONFIG:
      default:
        P2IE &= ~BIT1;
        P1IE &= ~BIT1;
        state = IDLE;
        break;
      }

      // Inicia debounce de S1
      TA1CTL |= TACLR;
      TA1CCTL1 |= CCIE;
      s1Pressed = 1;
    }
    break;
  default:
    break;
  }
}

#pragma vector = PORT1_VECTOR
/**
 * ISR de S2 (P1.1), disparada na borda de subida (ao soltar o botão).
 * O comportamento de S2 depende do estado atual:
 *   - Em WORK ou IDLE: desligar o alarme (TODO);
 *   - Em CONFIG: incrementa o limiar de alarme `threshold` em 5,
 *     voltando para 5 ao passar de 60 (ciclo 5..60).
 * Em seguida, inicia o debounce por software via Timer_A1/CCR2.
 */
__interrupt void s2() {
  switch (P1IV) {
  case P1IV_P1IFG1:
    if (!s2Pressed) { // Ignora a interrupção se o debounce já estiver ativo
      // Realiza operações de acordo com o estado
      switch (state) {
      case WORK:
      case IDLE:
        buzzerOff(); // Desliga alarme
        break;
      case CONFIG:
        threshold = threshold == 60 ? 5 : threshold + 5;
        break;
      default:
        break;
      }

      // Inicia debounce de S2
      TA1CTL |= TACLR;
      TA1CCTL2 |= CCIE;
      s2Pressed = 1;
    }
    break;
  default:
    break;
  }
}

#pragma vector = TIMER1_A1_VECTOR
/**
 * ISR do Timer_A1 (TIMER1_A1_VECTOR), responsável pelo debounce de S1
 * e S2. Disparada 20 ms após o pressionamento de cada botão (CCR1
 * para S1, CCR2 para S2); se o botão já estiver estável (nível alto =
 * solto, já que os pull-ups estão habilitados), encerra o debounce
 * limpando o respectivo flag de "pressionado". Caso contrário (ainda
 * em nível baixo / instável), reinicia a contagem de 20 ms.
 */
__interrupt void debounce() {
  switch (TA1IV) {
  case TA1IV_TA1CCR1: // Debounce de S1
    if (P2IN & BIT1) {
      TA1CCTL1 &= ~CCIE; // Desliga debounce
      s1Pressed = 0;
    } else
      TA1CTL |= TACLR; // Reinicia debounce
    break;
  case TA1IV_TA1CCR2: // Debounce de S1
    if (P1IN & BIT1) {
      TA1CCTL2 &= ~CCIE;
      s2Pressed = 0;
    } else
      TA1CTL |= TACLR;
    break;
  default:
    break;
  }
}
