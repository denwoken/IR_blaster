#include "Arduino.h"

void init_single_buffer();
void free_single_buffer();

void SwapBuffers()
{
}

void SendBufferr()
{
}
uint16_t *CurrBuffer();
void pixel_to_buff(int16_t x, int16_t y, int16_t color);
void rect_to_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color);