#include <adc.h>
#include <uart.h>
#include <lcd.h>
#include <sensor.h>

#include <stdio.h>

typedef enum State { IDLE, WORK, LCD_ERROR } State;

static State state = IDLE;

void setup() {
  WDTCTL = WDTPW | WDTHOLD; // Para watchdog timer

  setupADC12(0); // Usa canal 0 do ADC12 (P6.0)
  setupUART();

  if (!setupLCD())
    state = LCD_ERROR;

  __enable_interrupt(); // Habilita GIE
}

void main() {
  setup();
  static char read_buffer[32];
  int error_msg_printed = 0;
  
  State prev_state = IDLE;

  for (;;)
    switch (state) {
    case WORK:
      if (prev_state != state) {
        clearLCD();
        LCDWrite("Umidade: ", 0x00);
      }

      parseRead(read_buffer);

      // Para impressão de leituras no monitor serial
      // char debug_buffer[32];
      // sprintf(debug_buffer, "ADC: %d", analog_read_value);
      // serialPrintLn(debug_buffer);

      LCDWrite(read_buffer, 0x09);
      state = IDLE;
      prev_state = WORK;
      break;
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
    state = WORK;
    __low_power_mode_off_on_exit();
    break;
  default:
    break;
  }
}
