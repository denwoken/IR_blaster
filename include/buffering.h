#include "Arduino.h"

void init_single_buffer();
void init_double_buffer();
void free_all_buffers();

void SwapBuffers();
void SendBufferr();

void pixel_to_buff(int16_t x, int16_t y, int16_t color);
void rect_to_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color);
void char_to_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size);
void clear_buff();