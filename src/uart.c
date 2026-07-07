/**
 * uart.c
 *
 * Comunicação serial (USCI_A1) usada para depuração: impressão de
 * mensagens e da leitura bruta do ADC.
 */

#include <uart.h>

/**
 * Configura a USCI_A1 em modo UART para comunicação serial de
 * depuração: pinos P4.4 (TX) e P4.5 (RX), clock fonte ACLK
 * (32.768 kHz) e baud rate de 9600 bps (UCBR0=3, UCBR1=0, modulação
 * UCBRSx=0x06, conforme a tabela de configuração da USCI para esse
 * par clock/baud rate).
 */
void setupUART() {
  // Configurar pinos P4.4 (TX) e P4.5 (RX)
  P4SEL |= BIT4 | BIT5;

  // Configurar registradores UART
  UCA1CTL1 |= UCSWRST; // mantém a USCI em reset durante a configuração
  UCA1CTL1 |= UCSSEL__ACLK; // Uso de ACLK para BRCLK

  // Baud rate de 9600 com clock de 32.768Hz
  UCA1BR0 = 3;
  UCA1BR1 = 0;
  UCA1MCTL = 0x06;

  UCA1CTL1 &= ~UCSWRST; // Libera o USCI
}

/**
 * Envia uma string terminada em '\0' pela UART, byte a byte,
 * aguardando de forma bloqueante (polling) o flag UCTXIFG antes de
 * cada escrita no buffer de transmissão.
 */
void serialPrint(const char *str) {
  while (*str) {
    while (!(UCA1IFG & UCTXIFG))
      ; // Aguarda o buffer TX ser liberado
    UCA1TXBUF = *str++;
  }
}

/**
 * Envia uma string seguida por um terminador de linha (\r\n), útil
 * para separar mensagens de depuração no terminal serial.
 */
void serialPrintLn(const char *str) {
  serialPrint(str);
  serialPrint("\r\n");
}
