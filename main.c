#include <adc.h>
#include <uart.h>
#include <lcd.h>

#include <stdio.h>

#define DRY 0
#define DAMP 1800

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
  static char uart_buffer[32], lcd_buffer[80];
  volatile unsigned long humidity, humidity_perc;
  volatile unsigned int integer, decimal;
  int error_msg_printed = 0;

  for (;;)
    switch (mode) {
    case WORK:
      humidity = ((unsigned long)(4095 - analog_read_value) + 1) * 1000;
      humidity_perc = humidity >> 12; // Divide por 4096
      integer = humidity_perc / 10;
      decimal = humidity_perc % 10;

      sprintf(uart_buffer, "ADC: %d | Umidade: %d.%d%%", analog_read_value, integer,
              decimal);
      sprintf(lcd_buffer, "Umidade: %d.%d%%", integer, decimal);

      serialPrintLn(uart_buffer);

      clearLCD();
      LCDWrite(lcd_buffer);
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
