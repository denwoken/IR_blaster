#include "buffering.h"

#include "ST7735.h"
extern uint8_t _height;
extern uint8_t _width;
#include "Graphics.h"
extern Graphics tft;
#include "SPI.h"
extern "C"
{
#include <ets_sys.h>
#include "user_interface.h"
#include <osapi.h>
}
#include "FlashOptions.h"
extern Global_options Gl_options;

#define color_16_to_8(a) 0b111111 & (a >> 5)
#define color_8_to_16(a) (a << 5)

void *buff_ptr_0 = NULL;
void *buff_ptr_1 = NULL;
void *CurrBuffer = NULL;
void *SecondaryBuffer = NULL;

void free_all_buffers()
{
  if (buff_ptr_0 != NULL)
    os_free(buff_ptr_0);

  if (buff_ptr_1 != NULL)
    os_free(buff_ptr_1);

  buff_ptr_0 = NULL;
  buff_ptr_1 = NULL;
  CurrBuffer = NULL;
  SecondaryBuffer = NULL;
}

void init_single_buffer()
{
  free_all_buffers();

  buff_ptr_0 = os_malloc(_width * _height * 2);
  while (buff_ptr_0 == NULL)
  {
    Serial.println("bad alloc");
    delay(10);
  }
  os_memset(buff_ptr_0, 0, _width * _height * 2);
  CurrBuffer = buff_ptr_0;
}
void init_double_buffer()
{

  // SecondaryBuffer = (void *)((uint8_t *)buff_ptr + _width * _height );
  free_all_buffers();

  buff_ptr_0 = os_malloc(_width * _height * 1);
  buff_ptr_1 = os_malloc(_width * _height * 1);

  while (buff_ptr_0 == NULL || buff_ptr_1 == NULL)
  {
    Serial.println("bad alloc");
    delay(10);
  }
  os_memset(buff_ptr_0, 0, _width * _height * 1);
  os_memset(buff_ptr_1, 0, _width * _height * 1);
  CurrBuffer = buff_ptr_0;
  SecondaryBuffer = buff_ptr_1;
}

void SwapBuffers()
{
  void *a = CurrBuffer;
  CurrBuffer = SecondaryBuffer;
  SecondaryBuffer = a;
}

void Send_Single_Bufferr()
{
  ST7735_setAddrWindow(0, 0, _width, _height);
  setDataBits(512);
  for (uint16_t pixel_num = 0; pixel_num < 128 * 160; pixel_num += 32)
  {
    for (uint16_t i = 0; i < 16; i++)
    {
      const uint16_t *offset = (uint16 *)CurrBuffer + 2 * i + pixel_num;
      *(&SPI1W0 + i) = (*(offset + 1)) | ((*offset) << 16);
    }
    SPI1CMD |= SPIBUSY;
    while (SPI1CMD & SPIBUSY)
      asm volatile("NOP\n");
  }
}

void start_sending_second_Bufferr()
{
  return;
  static uint16_t pixel_num = 0;
  ST7735_setAddrWindow(0, 0, _width, _height);
  setDataBits(512);

  pixel_num = 0;
  // while (pixel_num <= 128 * 160)
  {
    for (uint16_t i = 0; i < 16; i++)
    {
      const uint8_t *offset = (uint8 *)SecondaryBuffer + 2 * i + pixel_num;
      const uint16_t data0 = color_8_to_16(*(offset + 1));
      const uint16_t data1 = color_8_to_16(*offset);

      *(&SPI1W0 + i) = data0 | (data1 << 16);
    }
    SPI1CMD |= SPIBUSY;
    pixel_num += 32;

    while (SPI1CMD & SPIBUSY)
      asm volatile("NOP\n");
  }
}

void SendBufferr()
{
  switch (Gl_options.buffering)
  {
  case 0:
    break;

  case 1:
    Send_Single_Bufferr();

  case 2:
    // SwapBuffers();
    // start_sending_second_Bufferr();
    break;
  }
}

void pixel_to_buff(int16_t x, int16_t y, int16_t color)
{
  if (Gl_options.buffering == 1)
    ((uint16 *)CurrBuffer)[x * _height + y] = color;
  else
    ((uint8 *)CurrBuffer)[x * _height + y] = color_16_to_8(color);
}

void clear_buff()
{
  switch (Gl_options.buffering)
  {
  case 0:
    break;

  case 1:
    os_memset(CurrBuffer, 0, _height * _width * 2);
    break;

  case 2:
    os_memset(CurrBuffer, 0, _height * _width * 1);
    break;
  }
}

void rect_to_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color)
{
  if (Gl_options.buffering == 1)
  {
    if (color == 0)
    {
      for (uint16_t x = x0; x < x0 + w; x++)
        os_memset((uint16_t *)CurrBuffer + x * _height + y0, 0, h * 2);
      return;
    }

    for (uint16_t x = x0; x < x0 + w; x++)
    {
      for (uint16_t y = y0; y < y0 + h; y++)
      {
        ((uint16 *)CurrBuffer)[x * _height + y] = color;
      }
    }
  }
  else
  {
    uint8_t color_8 = color_16_to_8(color);

    for (uint16_t x = x0; x < x0 + w; x++)
      os_memset((uint8 *)CurrBuffer + x * _height + y0, color_8, h);
  }
}

#include "glcdfont.c"
void char_to_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size)
{

  uint8_t char_m[6];
  for (uint8_t i = 0; i < 5; i++) // load char to stack
    char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
  char_m[5] = 0;

  if (Gl_options.buffering == 1)
  {
    switch (size)
    {
    case 1:
      for (uint16_t dx = 0; dx < 6; dx++)
      {
        for (uint16_t dy = 0; dy < 8; dy++)
        {
          ((uint16 *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx] >> (dy))) ? color : bg;
        }
      }
      break;
    case 2:
      for (uint16_t dx = 0; dx < 6 * 2; dx++)
      {
        for (uint16_t dy = 0; dy < 8 * 2; dy++)
        {
          ((uint16 *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx >> 1] >> (dy >> 1))) ? color : bg;
        }
      }
      break;
    case 3:
      for (uint16_t dx = 0; dx < 6 * 3; dx++)
      {
        for (uint16_t dy = 0; dy < 8 * 3; dy++)
        {
          ((uint16 *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx / 3] >> (dy / 3))) ? color : bg;
        }
      }
      break;
    }
  }
  else
  {
    uint8_t color_8 = color_16_to_8(color);
    uint8_t bg_8 = color_16_to_8(bg);
    switch (size)
    {
    case 1:
      for (uint16_t dx = 0; dx < 6; dx++)
      {
        for (uint16_t dy = 0; dy < 8; dy++)
        {
          ((uint8_t *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx] >> (dy))) ? color_8 : bg_8;
        }
      }
      break;
    case 2:
      for (uint16_t dx = 0; dx < 6 * 2; dx++)
      {
        for (uint16_t dy = 0; dy < 8 * 2; dy++)
        {
          ((uint8_t *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx >> 1] >> (dy >> 1))) ? color_8 : bg_8;
        }
      }
      break;
    case 3:
      for (uint16_t dx = 0; dx < 6 * 3; dx++)
      {
        for (uint16_t dy = 0; dy < 8 * 3; dy++)
        {
          ((uint8_t *)CurrBuffer)[(y0 + dy) + _height * (x0 + dx)] =
              (1 & (char_m[dx / 3] >> (dy / 3))) ? color_8 : bg_8;
        }
      }
      break;
    }
  }
}