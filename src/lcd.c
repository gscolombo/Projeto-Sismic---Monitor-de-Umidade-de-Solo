/**
 * lcd.c
 *
 * Driver do display LCD (controlador HD44780 compatível) via expansor
 * I2C PCF8574, incluindo a sequência de inicialização, escrita/leitura
 * de nibbles e bytes, e funções de alto nível para escrever texto e
 * limpar o display.
 */

#include <lcd.h>

/**
 * Inicializa o display LCD via PCF8574, seguindo a sequência padrão
 * de inicialização do HD44780 em modo 4 bits:
 *   1. Envia 0x30 três vezes (com atrasos) para forçar o LCD ao modo
 *      8 bits, independentemente do seu estado anterior;
 *   2. Comuta para o modo 4 bits (0x20);
 *   3. Configura 2 linhas (0x28);
 *   4. Liga o display com cursor estático visível (0x0C);
 *   5. Limpa o display e retorna o cursor ao início (0x01).
 *
 * Cada etapa só é executada se a etapa anterior teve sucesso (`ack`).
 * Retorna 1 se todas as etapas tiveram ACK do PCF8574/LCD, ou 0 se
 * alguma falhou.
 */
uint8_t setupLCD() {
  uint8_t ack = 1, i;

  setupI2C();

  // Aguarda inicialização do LCD
  __delay_cycles(20000);

  // Envia 0x30 3 vezes para garantir o LCD no modo 8 bits
  for (i = 3; i != 0; i--)
    if (ack) {
      LCDWriteNibble(0x30, 0);
      __delay_cycles(5000);
    }

  // Configura LCD no modo 4 bits
  if (ack) {
    ack = LCDWriteNibble(0x20, 0);
    while (LCDBusy())
      ;
  }

  // Configura modo 2 linhas
  if (ack) {
    ack = LCDWriteByte(0x28, 0);
    while (LCDBusy())
      ;
  }

  // Liga display | Liga o cursor (estático)
  if (ack) {
    ack = LCDWriteByte(0x0C, 0);
    while (LCDBusy())
      ;
  }

  // Limpa o conteúdo do display e retorna cursor para o início
  if (ack) {
    ack = LCDWriteByte(0x01, 0);
    while (LCDBusy())
      ;
  }

  return ack;
}

/**
 * Envia um nibble (4 bits, em D[7:4] de `nibble`) ao LCD via PCF8574,
 * pulsando o sinal EN (Enable): escreve primeiro os dados com EN=0,
 * depois com EN=1 e por fim novamente com EN=0, conforme exigido pelo
 * protocolo de escrita do HD44780. `isChar` define se o nibble é dado
 * (RS=1) ou comando (RS=0) — esse bit é embutido no próprio valor de
 * `isChar` (LCD_RS = 0x01).
 *
 * Retorna 1 se todas as transmissões I2C subjacentes tiveram ACK.
 */
uint8_t LCDWriteNibble(uint8_t nibble, uint8_t isChar) {
  uint8_t data, ack = 0;

  // nibble em D[7:4], EN = 0, R/W = 0 e RS = isChar
  data = nibble | LCD_BL | isChar;
  ack = i2cSend(LCD_I2C_ADDR, data);
  if (ack)
    ack = i2cSend(LCD_I2C_ADDR, data | LCD_EN); // EN = 1;

  if (ack)
    ack = i2cSend(LCD_I2C_ADDR, data & ~LCD_EN); // EN = 0;

  return ack;
}

/**
 * Envia um byte completo ao LCD em modo 4 bits, transmitindo primeiro
 * o nibble mais significativo (MSB, D[7:4]) e depois o menos
 * significativo (LSB, deslocado para D[7:4]), cada um via
 * LCDWriteNibble(). `isChar` indica se o byte é dado (caractere) ou
 * comando, conforme LCDWriteNibble().
 */
uint8_t LCDWriteByte(uint8_t byte, uint8_t isChar) {
  uint8_t ack = 0;

  ack = LCDWriteNibble(byte & 0xF0, isChar); // Envia MSB
  if (ack)
    ack = LCDWriteNibble((byte & 0x0F) << 4, isChar); // Envia LSB

  return ack;
}

/**
 * Lê um nibble do LCD via PCF8574, com R/W=1 (leitura) e D[7:4] em
 * alta impedância (0xF0) para permitir que o LCD conduza o barramento.
 * O nibble lido (D[7:4] do byte recebido) é retornado em `*buf`
 * (mascarado para manter apenas os 4 bits superiores).
 */
uint8_t LCDReadNibble(uint8_t *buf, uint8_t isChar) {
  uint8_t ack = 0, data = 0xF0 | LCD_BL | LCD_RW | isChar;

  ack = i2cSend(LCD_I2C_ADDR, data);
  if (ack)
    ack = i2cSend(LCD_I2C_ADDR, data | LCD_EN); // EN = 1

  if (ack) {
    ack = i2cReceive(LCD_I2C_ADDR, buf);
    *buf &= 0xF0;
  }

  if (ack)
    ack = i2cSend(LCD_I2C_ADDR, data & ~LCD_EN); // EN = 0

  return ack;
}

/**
 * Lê um byte completo do LCD (ex.: registrador de status / busy flag),
 * combinando dois nibbles lidos via LCDReadNibble(): o primeiro
 * nibble lido corresponde aos 4 bits superiores de `*byte` e o
 * segundo aos 4 bits inferiores.
 *
 * Observação: `*byte` deve estar zerado antes da chamada, já que os
 * bits são combinados com OR (|=).
 */
uint8_t LCDReadByte(uint8_t *byte, uint8_t isChar) {
  uint8_t ack = 0;
  uint8_t buf;

  ack = LCDReadNibble(&buf, isChar);
  if (ack) {
    *byte |= buf & 0xF0;
    ack = LCDReadNibble(&buf, isChar);
  }

  if (ack)
    *byte |= buf >> 4;

  return ack;
}

/**
 * Escreve a string `str` no LCD a partir da posição `pos` (endereço de
 * DDRAM, ex.: 0x00 = início da linha 1, 0x40 = início da linha 2).
 * Caracteres de quebra de linha ('\n') alternam o cursor entre a
 * linha 1 (0x00) e a linha 2 (0x40), em vez de serem impressos como
 * dado. Um pequeno atraso (`__delay_cycles(50)`) é inserido após cada
 * caractere/comando para respeitar o tempo de execução do LCD.
 *
 * Interrompe a escrita assim que um comando falha (sem ACK) ou ao
 * atingir o fim da string. Retorna 1 se toda a string foi escrita com
 * sucesso.
 */
uint8_t LCDWrite(char *str, uint8_t pos) {
  uint8_t ack = 1;
  char *c = str;
  static uint8_t ln = 0x00; // Memoriza a linha atual entre chamadas, para tratar '\n'

  // Posiciona o cursor
  LCDWriteByte(0x80 | (pos & 0x7F), 0);

  while (ack && *c != 0x00) {
    if (*c == '\n') {
      ln ^= 0x40; // Alterna entre linhas a cada quebra de linha
      ack = LCDWriteByte(0x80 | ln, 0);
      c++;
    } else
      ack = LCDWriteByte(*c++, 1);
    __delay_cycles(50);
  }

  return ack;
}

/**
 * Lê o registrador de status do LCD (via LCDReadByte) e retorna o bit
 * de "busy flag" (bit 7), indicando se o controlador ainda está
 * processando o último comando.
 */
uint8_t LCDBusy() {
  uint8_t bf = 0;
  LCDReadByte(&bf, 0);

  bf &= 0x80;
  bf >>= 7;

  return bf;
}

/** Limpa todo o conteúdo do display e aguarda o tempo de execução do comando. */
void clearLCD() {
  LCDWriteByte(0x01, 0);
  __delay_cycles(20000);
}

/**
 * Limpa apenas uma linha do display (preenchendo com 16 espaços), sem
 * afetar a outra linha: n=0 limpa a linha 1 (posição 0x00); qualquer
 * valor diferente de zero limpa a linha 2 (posição 0x40).
 */
void clearLine(int n) {
  if (n)
    LCDWrite("                ", 0x40); // 16 caracteres de espaço
  else
    LCDWrite("                ", 0x00); // 16 caracteres de espaço
}
