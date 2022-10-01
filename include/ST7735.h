
#pragma once

#include "Arduino.h"

#include "SPI.h"

// uint32_t get_err();
#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

#define ST7735_TFTWIDTH_128 128
#define ST7735_TFTHEIGHT_160 160 // for 1.8" display
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00

enum
{
	NONE,
	FILL_RECT_COMM,
	BUFF_COMM,
	CHAR_COMM_1,
	CHAR_COMM_2,
};
/*
#define FILL_RECT_COMM 2
#define BUFF_COMM 3
#define CHAR_COMM_1 4
#define CHAR_COMM_2 5
*/

#define BUFF_SIZE 100

#define ST7735_setAddr(x, w) ((x << 16) | (x + w - 1))
#define SPI_INTR_ENABLE() \
	;                     \
	ETS_SPI_INTR_ENABLE();
#define SPI_INTR_DISABLE() \
	;                      \
	ETS_SPI_INTR_DISABLE();

void ST7735_begin_spi_intr();

void ST7735_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
// inline uint32_t ST7735_setAddr(uint16_t x, uint16_t w);

// void ST7735_setfreq(uint32_t freq);
void ST7735_begin(int8_t dc, uint32_t freq);

void ST7735_invertDisplay(bool i);
void ST7735_setRotation(uint8_t r);

void ST7735_sendCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);
void ST7735_sendCommand(uint8_t commandByte, const uint8_t *dataBytes = NULL, uint8_t numDataBytes = 0);
void ST7735_sendCommand16(uint16_t commandWord, const uint8_t *dataBytes = NULL, uint8_t numDataBytes = 0);
void writeCommand(uint8_t cmd);
void writeCommand16(uint16_t cmd);

void ST7735_displayInit(const uint8_t *addr);

void wait_end_sending();

void ST7735_pixel_to_queue(int16_t x, int16_t y, uint16_t color);
void ST7735_Rect_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void ST7735_buff_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color);
void ST7735_char_to_queue(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size);

bool queue_is_Nempty();
//#define queue_is_Nempty() spi_intr_flag
void wait_queue_to_empty();
//#define
