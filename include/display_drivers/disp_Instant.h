#pragma once
#include "Arduino.h"

namespace ST7735
{
    void IRAM_ATTR draw_pixel(uint16_t x, uint16_t y, uint16_t color);

    void IRAM_ATTR draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    void IRAM_ATTR draw_Bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *colors);

    void IRAM_ATTR draw_char(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size);
}