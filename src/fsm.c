#include <fsm.h>

State state = START, prev_state = START;
uint16_t threshold = 50;

uint8_t s1Pressed = 0, s2Pressed = 0;

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
__interrupt void checkRead() {
  switch (TA0IV) {
  case TA0IV_TA0IFG:
    P6OUT |= BIT5;

    if (state == START) {
      state = WORK;
      prev_state = START;
      __low_power_mode_off_on_exit();
    }

    if (state == IDLE) {
      state = WORK;
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

// Interrupção de S1 acionada na borda de subida (ao soltar o botão)
#pragma vector = PORT2_VECTOR
__interrupt void s1() {
  switch (P2IV) {
  case P2IV_P2IFG1:
    if (!s1Pressed) {
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
__interrupt void s2() {
  switch (P1IV) {
  case P1IV_P1IFG1:
    if (!s2Pressed) {
      // Realiza operações de acordo com o estado
      switch (state) {
      case WORK:
      case IDLE:
        // Desliga alarme
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
__interrupt void debounce() {
  switch (TA1IV) {
  case TA1IV_TA1CCR1:
    if (P2IN & BIT1) {
      TA1CCTL1 &= ~CCIE; // Desliga debounce
      s1Pressed = 0;
    } else
      TA1CTL |= TACLR; // Reinicia debounce
    break;
  case TA1IV_TA1CCR2:
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
