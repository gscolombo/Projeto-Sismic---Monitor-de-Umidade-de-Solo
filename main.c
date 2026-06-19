#include <adc.h>
#include <fsm.h>
#include <lcd.h>
#include <sensor.h>
#include <uart.h>

#include <stdio.h>

#define DEBUG 1

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
  else
    clearLCD();

  // Configura pinos P2.1 (S1) and P1.1 (S2) para configuração do sistema
  setupSwitches();

  __enable_interrupt(); // Habilita GIE
}

void main() {
  setup();

  static char lcd_buffer[80];
  int error_msg_printed = 0;
  Percent p;
  static uint8_t setup_mode = 1;

  for (;;)
    switch (state) {
    case WORK:
      if (prev_state == START) {
        clearLCD();
        LCDWrite("Iniciando...", 0x00);
        state = IDLE;
        prev_state = WORK;
        break;
      }

      if (setup_mode) {
        clearLCD();
        LCDWrite("Umidade: ", 0x00);
        setup_mode = 0;
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
        // Exibe conteúdo inicial (configuração de limite de umidade)
        clearLCD();
        LCDWrite("Nível mínimo", 0x00);
        LCDWrite("Umidade: ", 0x40);
        P6OUT &= ~BIT5;
        setup_mode = 1;
      }

      sprintf(lcd_buffer, "%d.0%%   ", threshold);
      LCDWrite(lcd_buffer, 0x49);
      prev_state = CONFIG;
      break;
    case IDLE:
      if (prev_state == CONFIG) {
        clearLCD();
        LCDWrite("Reiniciando...", 0x00);
        TA0CTL &= ~TAIFG;
        TA0CTL |= TAIE | TACLR;
      }

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
