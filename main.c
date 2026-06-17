#include <adc.h>
#include <config.h>
#include <lcd.h>
#include <sensor.h>
#include <uart.h>

#include <stdio.h>

#define DEBUG 1

typedef enum State { IDLE, WORK, CONFIG, LCD_ERROR } State;

static State state = IDLE;

void printRead() {
  char debug_buffer[32];
  sprintf(debug_buffer, "ADC: %d", value);
  serialPrintLn(debug_buffer);
}

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
  static char lcd_buffer[80];
  int error_msg_printed = 0;
  Percent p;

  State prev_state = IDLE;

  for (;;)
    switch (state) {
    case WORK:
      if (prev_state != state) {
        clearLCD();
        LCDWrite("Umidade: ", 0x00);
      }

      p = convertRead();
      sprintf(lcd_buffer, "%d.%d%%   ", p.i, p.d);

      if (DEBUG)
        printRead();

      LCDWrite(lcd_buffer, 0x09);
      checkThreshold(&p, threshold);

      state = IDLE;
      prev_state = WORK;
      break;
    case CONFIG:

      if (prev_state != state) {
        // Pausa leitura de sensor

        // Exibe conteúdo inicial (configuração de limite de umidade)
        clearLCD();
        LCDWrite("Config.", 0x00);
        LCDWrite("Nível mínimo: ", 0x40);
      }

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
    P6OUT |= BIT5;

    if (state == IDLE) {
      state = WORK;
      __low_power_mode_off_on_exit();
    }

    if (state == CONFIG) {
    }

    break;
  default:
    break;
  }
}
