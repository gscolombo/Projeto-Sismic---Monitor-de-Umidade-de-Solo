/**
 * i2c.c
 *
 * Driver mínimo de I2C mestre (USCI_B0), usado para comunicação com o
 * expansor PCF8574 que controla o LCD.
 */

#include <i2c.h>

/**
 * Configura a USCI_B0 em modo I2C mestre, síncrono, usando SMCLK
 * (1 MHz) como clock fonte e gerando SCL em 100 kHz (UCB0BR0 = 10).
 * Os pinos P3.0 (SDA) e P3.1 (SCL) são roteados para a função do
 * periférico USCI_B0.
 */
void setupI2C() {
  UCB0CTL1 |= UCSWRST;                  // Reseta para iniciar a configuração
  UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC; // Mestre, I2C (MODE = 3), síncrono
  UCB0CTL1 |= UCSSEL_2;                 // Usa SMCLK (1 MHz)

  // Frequência de SCL = SMCLK/10 = 100 kHz
  UCB0BR0 = 10;
  UCB0BR1 = 0;

  // Configura pinos P3.0 (SDA) e P3.1 (SCL) para barramento I²C
  P3SEL = BIT0 | BIT1;

  // Finaliza a configuração
  UCB0CTL1 &= ~UCSWRST;
}

/**
 * Transmite um único byte `data` para o escravo I2C de endereço
 * `addr`, em uma transação completa START -> endereço -> dado -> STOP,
 * de forma bloqueante (polling dos flags da USCI_B0).
 *
 * Retorna 1 (sucesso) se nenhum NACK foi recebido durante a
 * transmissão, ou 0 caso o escravo tenha respondido com NACK.
 */
uint8_t i2cSend(uint8_t addr, uint8_t data) {
  UCB0I2CSA = addr; // Definição do endereço do escravo

  // Requisição de START como transmissor
  UCB0CTL1 |= UCTR;    // Modo de transmissão
  UCB0CTL1 |= UCTXSTT; // Gera condição START

  // Aguarda buffer de transmissão estar disponível
  while (!(UCB0IFG & UCTXIFG))
    ;

  UCB0TXBUF = data; // Escreve dados para transmissão no buffer

  while (UCB0CTL1 & UCTXSTT) // Aguarda START e endereço serem transmitidos.
    ;

  if (!(UCB0IFG & UCNACKIFG)) {  // Verifica resposta do escravo (ACK)
    while (!(UCB0IFG & UCTXIFG)) // Aguarda dados serem transmitidos
      ;
  }

  UCB0CTL1 |= UCTXSTP;       // Gera condição STOP
  while (UCB0CTL1 & UCTXSTP) // Aguarda transmissão de STOP
    ;

  return !(UCB0IFG & UCNACKIFG); // Verifica resposta do escravo (ACK);
}

/**
 * Recebe um único byte do escravo I2C de endereço `addr` e o armazena
 * em `*buf`, em uma transação START -> endereço (leitura) -> dado ->
 * STOP. Como apenas um byte é lido, o STOP é solicitado imediatamente
 * após o START (gerando NACK automático ao escravo após esse byte).
 *
 * Retorna 1 (sucesso, byte armazenado em `*buf`) se o escravo
 * respondeu com ACK ao endereço, ou 0 em caso de NACK.
 */
uint8_t i2cReceive(uint8_t addr, uint8_t *buf) {
  UCB0I2CSA = addr; // Definição do endereço do escravo

  UCB0CTL1 &= ~UCTR;   // Modo de receptor
  UCB0CTL1 |= UCTXSTT; // Geração de condição START

  while (UCB0CTL1 & UCTXSTT) // Aguarda START e endereço serem transmitidos.
    ;

  UCB0CTL1 |= UCTXSTP; // Gera uma condição STOP para receber somente um byte

  uint8_t ack = !(UCB0IFG & UCNACKIFG); // Verifica sinal ACK do escravo

  if (ack) {
    while (!(UCB0IFG & UCRXIFG))
      ;
 
    *buf = UCB0RXBUF;
  }

  while (UCB0CTL1 & UCTXSTP) // Aguarda transmissão de STOP
    ;

  return ack;
}
