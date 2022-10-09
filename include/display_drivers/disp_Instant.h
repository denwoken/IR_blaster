#pragma once
#include "Arduino.h"

namespace ST7735
{
    void draw_pixel(uint16_t x, uint16_t y, uint16_t color);

    void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    void draw_buff(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *colors);

    void draw_char(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size);
}