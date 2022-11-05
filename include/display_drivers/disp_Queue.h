#pragma once
#include "Arduino.h"
namespace disp_Queue
{
    enum
    {
        NONE,
        FILL_RECT_COMM,
        BUFF_COMM,
        CHAR_COMM_1,
        CHAR_COMM_2,
    };

#define BUFF_SIZE 100

    void init_display_Queue();

    void IRAM_ATTR pixel_to_queue(int16_t x, int16_t y, uint16_t color);
    void IRAM_ATTR Rect_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void IRAM_ATTR Bitmap_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color, uint8_t free_flag);
    void IRAM_ATTR char_to_queue(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size);
}

bool IRAM_ATTR queue_is_Nempty();
//#define queue_is_Nempty() spi_intr_flag
void IRAM_ATTR wait_queue_to_empty();

void IRAM_ATTR wait_end_sending();