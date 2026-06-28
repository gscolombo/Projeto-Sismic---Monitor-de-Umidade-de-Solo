/**
 * main.c
 *
 * Loop principal e máquina de estados (FSM) do firmware de
 * monitoramento de umidade do solo. Integra os módulos de ADC
 * (leitura do sensor), UART (depuração serial), LCD (interface com o
 * usuário) e botões (configuração do limiar de alarme).
 */

#include <adc.h>
#include <fsm.h>
#include <lcd.h>
#include <sensor.h>
#include <uart.h>

#include <stdio.h>

#define DEBUG 1

/**
 * Monta uma string com a última leitura bruta do ADC (variável global
 * `value`, atualizada pela ISR do ADC12 em adc.c) e a envia pela UART
 * para fins de depuração.
 */
void printRead() {
  char debug_buffer[32];
  sprintf(debug_buffer, "ADC: %d", value);
  serialPrintLn(debug_buffer);
}

/**
 * Inicialização do sistema, executada uma única vez antes do loop
 * principal: desliga o watchdog, configura os periféricos (ADC12,
 * UART, LCD via I2C, botões de configuração) e habilita as
 * interrupções globais (GIE).
 *
 * Se a inicialização do LCD falhar, o sistema entra diretamente no
 * estado LCD_ERROR.
 */
void setup() {
  WDTCTL = WDTPW | WDTHOLD; // Para watchdog timer

  setupADC12(); // Usa canal 0 do ADC12 (P6.0)
  setupUART();

  if (!setupLCD())
    state = LCD_ERROR;
  else
    clearLCD();

  // Configura pinos P2.1 (S1) and P1.1 (S2) para configuração do sistema
  setupSwitches();

  __enable_interrupt(); // Habilita GIE
}

/**
 * Loop principal do firmware, implementado como uma máquina de estados
 * (FSM) com quatro estados:
 *   - WORK:      realiza uma leitura de umidade e atualiza o LCD;
 *   - CONFIG:    permite ajustar o limiar de alarme (threshold) via S2;
 *   - IDLE:      aguarda em modo de baixo consumo até a próxima
 *                conversão do ADC (acionada pelo TimerA0) ou até um
 *                botão ser pressionado;
 *   - LCD_ERROR: estado de falha, caso o LCD não tenha sido
 *                inicializado corretamente.
 *
 * As transições de estado e a variável `prev_state` (estado anterior)
 * são usadas para detectar a primeira execução de um estado e desenhar
 * o conteúdo estático do LCD apenas uma vez por entrada no estado.
 */

void main() {
  setup();

  static char lcd_buffer[80];
  int error_msg_printed = 0;
  Percent p;
  static uint8_t setup_mode =
      1; // controla a impressão única do rótulo "Umidade:" no LCD

  for (;;)
    switch (state) {
    case WORK:
      // Primeira transição START -> WORK: apenas exibe a mensagem de
      // boas-vindas e vai para IDLE, sem realizar leitura ainda.
      if (prev_state == START) {
        clearLCD();
        LCDWrite("Iniciando...", 0x00);
        state = IDLE;
        prev_state = WORK;
        break;
      }

      // Exibe o rótulo fixo "Umidade:" apenas na primeira vez que o
      // sistema entra em modo de trabalho (após CONFIG ou START).
      if (setup_mode) {
        clearLCD();
        LCDWrite("Umidade: ", 0x00);
        setup_mode = 0;
      }

      // Converte a última leitura do ADC em percentual de umidade e
      // exibe no LCD (posição 0x09, mesma linha do rótulo "Umidade:").
      p = convertRead();
      sprintf(lcd_buffer, "%d.%d%%   ", p.i, p.d);

      if (DEBUG)
        printRead(); // Envia a leitura bruta via UART para depuração

      LCDWrite(lcd_buffer, 0x09);
      checkThreshold(&p, threshold); // Atualiza o alarme visual, se necessário

      state = IDLE;
      prev_state = WORK;
      break;
    case CONFIG:

      // Primeira vez no estado CONFIG: desenha o layout estático
      // (rótulos) e desliga a alimentação do sensor, já que não há
      // leituras durante a configuração.
      if (prev_state != state) {
        // Exibe conteúdo inicial (configuração de limite de umidade)
        clearLCD();
        LCDWrite("Nível mínimo", 0x00);
        LCDWrite("Umidade: ", 0x40);
        P6OUT &= ~BIT5;

        // Força reimpressão do rótulo "Umidade:" ao sair de CONFIG
        setup_mode = 1;
      }

      // Atualiza o valor de threshold exibido a cada iteração (pode
      // ter sido alterado por S2 dentro da ISR de botão).
      sprintf(lcd_buffer, "%d.0%%   ", threshold);
      LCDWrite(lcd_buffer, 0x49);
      prev_state = CONFIG;
      break;
    case IDLE:
      // Ao retornar de CONFIG, reinicia o ciclo do TimerA0 (conversões
      // periódicas do ADC)
      if (prev_state == CONFIG) {
        clearLCD();
        LCDWrite("Reiniciando...", 0x00);
        TA0CTL &= ~TAIFG;
        TA0CTL |= TAIE | TACLR;
      }

      __low_power_mode_1(); // Dorme até a próxima interrupção (ADC, S1 ou S2)
      break;
    case LCD_ERROR:
      // Estado terminal: reporta a falha de inicialização do LCD pela
      // UART uma única vez.
      if (!error_msg_printed) {
        serialPrintLn("Erro na inicialização do LCD.");
        error_msg_printed = 1;
      }
      break;
    default:
      break;
    }
}
