#include <adc.h>
#include <uart.h>

#include <stdio.h>

typedef enum state { IDLE, WORK } state;

static state mode = IDLE;

void setup() {
  WDTCTL = WDTPW | WDTHOLD; // Para watchdog timer

  setupADC12(0); // Usa canal 0 do ADC12 (P6.0)
  setupUART();

  __enable_interrupt(); // Habilita GIE
}

void main() {
  setup();
  static char buffer[32];
  volatile unsigned long humidity, humidity_perc;
  volatile unsigned int integer, decimal;

  for (;;)
    switch (mode) {
    case WORK:
      humidity = ((long)(4095 - analog_read_value) + 1) * 1000;
      humidity_perc = humidity >> 12; // Divide por 4096
      integer = humidity_perc / 10;
      decimal = humidity_perc % 10;

      sprintf(buffer, "ADC: %d | Umidade: %d.%d%%", analog_read_value, integer, decimal);
      serialPrintLn(buffer);
      mode = IDLE;
    case IDLE:
      __low_power_mode_1();
      break;
    default:
      break;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void checkRead() {

  switch (TA0IV) {
  case TA0IV_TA0IFG:
    mode = WORK;
    __low_power_mode_off_on_exit();
    break;
  default:
    break;
  }
}
