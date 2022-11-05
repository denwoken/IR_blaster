#include "Arduino.h"
#pragma once

#define color_16_to_8(a) Graphics::color565_to_gray8(a)
#define color_8_to_16(a) Graphics::gray8_to_color565(a)
//#define color_16_to_8(a) 0b111111 & (a >> 5)
//#define color_8_to_16(a) (a << 5)

namespace Buffering
{
    void IRAM_ATTR free_all_buffers();
    void IRAM_ATTR clear_buff();
    //
    //
    //
    void IRAM_ATTR init_single_buffer();
    void IRAM_ATTR Send_single_Bufferr();

    void IRAM_ATTR pixel_to_16bit_buff(uint16_t x, uint16_t y, uint16_t color);
    void IRAM_ATTR rect_to_16bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    void IRAM_ATTR char_to_16bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size);
    void IRAM_ATTR Bitmap_to_16bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t *colors);
    //
    //
    //
    void IRAM_ATTR init_double_buffer();
    void IRAM_ATTR init_buff_intr();
    void IRAM_ATTR wait_end_buffer_sending();
    void IRAM_ATTR stop_buffer_sending();
    void IRAM_ATTR SwapBuffers();
    void IRAM_ATTR Start_sending_double_Bufferr();

    void IRAM_ATTR pixel_to_8bit_buff(uint16_t x, uint16_t y, uint16_t color);
    void IRAM_ATTR rect_to_8bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    void IRAM_ATTR char_to_8bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size);
    void IRAM_ATTR Bitmap_to_8bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t *colors);

}