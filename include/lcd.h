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

/** Inicializa o LCD (modo 4 bits, 2 linhas). Retorna 1 em sucesso. */
uint8_t setupLCD();

/** Envia um nibble (comando se isChar=0, dado se isChar=1) ao LCD. */
uint8_t LCDWriteNibble(uint8_t, uint8_t);

/** Envia um byte completo (em dois nibbles) ao LCD. */
uint8_t LCDWriteByte(uint8_t, uint8_t);

/** Lê um nibble do LCD para *buf. */
uint8_t LCDReadNibble(uint8_t *, uint8_t);

/** Lê um byte completo (em dois nibbles) do LCD para *byte. */
uint8_t LCDReadByte(uint8_t *, uint8_t);

/** Retorna o busy flag do LCD (1 = ocupado). */
uint8_t LCDBusy();

/** Escreve uma string no LCD a partir da posição de DDRAM `pos`. */
uint8_t LCDWrite(char *, uint8_t);

/** Limpa todo o display. */
void clearLCD();

/** Limpa apenas a linha 1 (n=0) ou a linha 2 (n!=0) do display. */
void clearLine(int);

#endif
