#include <uart.h>

void setupUART() {
  // Configurar pinos P4.4 (TX) e P4.5 (RX)
  P4SEL |= BIT4 | BIT5;

  // Configurar registradores UART
  UCA1CTL1 |= UCSWRST;
  UCA1CTL1 |= UCSSEL__ACLK; // Uso de ACLK para BRCLK

  // Baud rate de 9600 com clock de 32.768Hz
  UCA1BR0 = 3;
  UCA1BR1 = 0;
  UCA1MCTL = 0x06;

  UCA1CTL1 &= ~UCSWRST; // Libera o USCI
}

void serialPrint(const char *str) {
  while (*str) {
    while (!(UCA1IFG & UCTXIFG))
      ; // Aguarda o buffer TX ser liberado
    UCA1TXBUF = *str++;
  }
}

void serialPrintLn(const char *str) {
  serialPrint(str);
  serialPrint("\r\n");
}
