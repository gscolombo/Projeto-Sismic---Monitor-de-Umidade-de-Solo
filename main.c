#include <adc.h>
#include <uart.h>
#include <lcd.h>
#include <sensor.h>

#include <stdio.h>

typedef enum state { IDLE, WORK, LCD_ERROR } state;

static state mode = IDLE;

void setup() {
  WDTCTL = WDTPW | WDTHOLD; // Para watchdog timer

  setupADC12(0); // Usa canal 0 do ADC12 (P6.0)
  setupUART();

  if (!setupLCD())
    mode = LCD_ERROR;

  __enable_interrupt(); // Habilita GIE
}

void main() {
  setup();
  static char read_buffer[32], debug_buffer[32];
  int error_msg_printed = 0;

  for (;;)
    switch (mode) {
    case WORK:
      parseRead(read_buffer);
      sprintf(debug_buffer, "ADC: %d", analog_read_value);

      serialPrintLn(debug_buffer);
      clearLCD();
      LCDWrite(read_buffer);
      mode = IDLE;
    case IDLE:
      __low_power_mode_1();
      break;
    case LCD_ERROR:
      if (!error_msg_printed) {
        serialPrintLn("Erro na inicialização do LCD.");
        error_msg_printed = 1;
      }
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
