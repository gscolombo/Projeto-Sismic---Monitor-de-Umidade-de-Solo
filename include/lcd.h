#ifndef LCD_H
#define LCD_H

#include <intrinsics.h>
#include <msp430.h>
#include <stdint.h>

#include <i2c.h>

#define LCD_I2C_ADDR 0x27 // Endereço do PCF8574
#define LCD_RS 0x01       // 0: Comando; 1: Dados
#define LCD_RW 0x02       // 0: Escrita; 1: Leitura
#define LCD_EN 0x04       // Enable
#define LCD_BL 0x08       // Luz de fundo (backlight)
#define LCD_DATA 0xF0     // D[7:4]

uint8_t setupLCD();

uint8_t LCDWriteNibble(uint8_t, uint8_t);
uint8_t LCDWriteByte(uint8_t, uint8_t);

uint8_t LCDReadNibble(uint8_t *, uint8_t);
uint8_t LCDReadByte(uint8_t *, uint8_t);
uint8_t LCDBusy();

uint8_t LCDWrite(char *, uint8_t);

void clearLCD();

#endif
