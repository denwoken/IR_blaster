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

    void pixel_to_queue(int16_t x, int16_t y, uint16_t color);
    void Rect_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void buff_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color);
    void char_to_queue(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size);
}

bool queue_is_Nempty();
//#define queue_is_Nempty() spi_intr_flag
void wait_queue_to_empty();

void wait_end_sending();