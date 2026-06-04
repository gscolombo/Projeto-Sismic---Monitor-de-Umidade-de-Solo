#include <lcd.h>

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

uint8_t LCDWriteByte(uint8_t byte, uint8_t isChar) {
  uint8_t ack = 0;

  ack = LCDWriteNibble(byte & 0xF0, isChar); // Envia MSB
  if (ack)
    ack = LCDWriteNibble((byte & 0x0F) << 4, isChar); // Envia LSB

  return ack;
}

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

uint8_t LCDWrite(char *str) {
  uint8_t ack = 1;
  char *c = str;
  static uint8_t ln = 0x00;

  while (ack && *c != 0x00 && !LCDBusy()) {
    if (*c == '\n') {
      ln ^= 0x40; // Alterna entre linhas a cada quebra de linha
      ack = LCDWriteByte(0x80 | ln, 0);
      c++;
    } else
      ack = LCDWriteByte(*c++, 1);
  }

  return ack;
}

uint8_t LCDBusy() {
  uint8_t bf = 0;
  LCDReadByte(&bf, 0);

  bf &= 0x80;
  bf >>= 7;

  return bf;
}

void clearLCD() {
  LCDWriteByte(0x01, 0);
  while (LCDBusy())
    ;
}
