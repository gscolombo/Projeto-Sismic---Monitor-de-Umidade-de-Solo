#include <adc.h>
#include <uart.h>

#include <stdio.h>

void setup() {
  WDTCTL = WDTPW | WDTHOLD; // Para watchdog timer

  setupADC12(0); // Usa canal 0 do ADC12 (P6.0)
  setupUART();

  __enable_interrupt(); // Habilita GIE
}

void main() {
  setup();

  for (;;)
    __low_power_mode_1();
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void checkRead() {
  static char buffer[32];

  switch (TA0IV) {
  case TA0IV_TA0IFG:
    sprintf(buffer, "ADC: %d", analog_read_value);
    serialPrintLn(buffer);
    break;
  default:
    break;
  }
}
