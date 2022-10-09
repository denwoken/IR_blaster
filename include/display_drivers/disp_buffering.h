#include "Arduino.h"
#pragma once
namespace Buffering
{
    void free_all_buffers();
    void clear_buff();
    //
    //
    //
    void init_single_buffer();
    void Send_single_Bufferr();

    void pixel_to_16bit_buff(int16_t x, int16_t y, int16_t color);
    void rect_to_16bit_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color);
    void char_to_16bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size);
    //
    //
    //
    void init_double_buffer();
    void init_buff_intr();
    void wait_end_buffer_sending();
    void stop_buffer_sending();
    void SwapBuffers();
    void Start_sending_double_Bufferr();

    void pixel_to_8bit_buff(int16_t x, int16_t y, int16_t color);
    void rect_to_8bit_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color);
    void char_to_8bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size);
}