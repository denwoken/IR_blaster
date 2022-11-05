#include "display_drivers/disp_buffering.h"
#include "display_drivers/driver_ST7735.h"
#include "display_drivers/SPI.h"

extern "C"
{
#include <ets_sys.h>
#include "user_interface.h"
#include <osapi.h>
}
#include "FlashOptions.h"

#include "Graphics.h"

// using namespace Buffering;

void *buff_ptr_0 = NULL;
void *buff_ptr_1 = NULL;
void *CurrBuffer = NULL;
void *SecondaryBuffer = NULL;

static bool sending_buff = 0;
static uint16_t pixel_num = 0;
static void IRAM_ATTR double_buff_spi_intr()
{
  WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
  if (pixel_num < 128 * 160)
  {
    for (uint16_t i = 0; i < 16; i++)
    {
      const uint8_t *offset = (uint8 *)SecondaryBuffer + 2 * i + pixel_num;
      const uint16_t data0 = Graphics::gray8_to_color565(offset[1]);
      const uint16_t data1 = Graphics::gray8_to_color565(offset[0]);
      *(&SPI1W0 + i) = data0 | (data1 << 16);
    }
    pixel_num += 32;
    SPI1CMD |= SPIBUSY;
    return;
  }
  SPI_INTR_DISABLE();
  sending_buff = 0;
}

void IRAM_ATTR Buffering::free_all_buffers()
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

void IRAM_ATTR Buffering::init_single_buffer()
{
  free_all_buffers();
  buff_ptr_0 = os_malloc(_width * _height * 2);
  while (buff_ptr_0 == NULL)
    tft.CriticalEror(PSTR("Critical Eror\nBad memory Allocationmemory is too\nfragmented.\nRestarting..."));

  os_memset(buff_ptr_0, 0, _width * _height * 2);
  CurrBuffer = buff_ptr_0;
}
void IRAM_ATTR Buffering::init_double_buffer()
{
  free_all_buffers();

  buff_ptr_0 = os_malloc(_width * _height * 1);
  buff_ptr_1 = os_malloc(_width * _height * 1);

  while (buff_ptr_0 == NULL || buff_ptr_1 == NULL)
    tft.CriticalEror(PSTR("Critical Eror\nBad memory Allocationmemory is too\nfragmented.\nRestarting..."));
  os_memset(buff_ptr_0, 0, _width * _height * 1);
  os_memset(buff_ptr_1, 0, _width * _height * 1);
  CurrBuffer = buff_ptr_0;
  SecondaryBuffer = buff_ptr_1;

  begin_spi_intr(double_buff_spi_intr);
}

void IRAM_ATTR Buffering::SwapBuffers()
{
  void *a = CurrBuffer;
  CurrBuffer = SecondaryBuffer;
  SecondaryBuffer = a;
}

void IRAM_ATTR Buffering::Send_single_Bufferr()
{
  ST7735::setAddrWindow(0, 0, _width, _height);
  setDataBits(512);
  for (uint16_t pixel_num = 0; pixel_num < 128 * 160; pixel_num += 32)
  {
    for (uint16_t i = 0; i < 16; i++)
    {
      const uint16_t *offset = (uint16 *)CurrBuffer + 2 * i + pixel_num;
      *(&SPI1W0 + i) = (*(offset + 1)) | ((*offset) << 16);
    }
    SPI1CMD |= SPIBUSY;
    WAIT_END_PACK_TRANSFER();
  }
}
void IRAM_ATTR Buffering::wait_end_buffer_sending()
{
  while (sending_buff)
    asm volatile("NOP\nNOP\n");
}
void IRAM_ATTR Buffering::stop_buffer_sending()
{
  wait_end_buffer_sending();
  SPI_INTR_DISABLE();
  sending_buff = 0;
  WAIT_END_PACK_TRANSFER();
}

void IRAM_ATTR Buffering::Start_sending_double_Bufferr()
{
  wait_end_buffer_sending();

  ST7735::setAddrWindow(0, 0, _width, _height);
  setDataBits(512);
  pixel_num = 0;
  sending_buff = 1;
  WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
  SPI_INTR_ENABLE();
}

void IRAM_ATTR Buffering::pixel_to_16bit_buff(uint16_t x, uint16_t y, uint16_t color)
{
  ((uint16 *)CurrBuffer)[x * _height + y] = color;
}
void IRAM_ATTR Buffering::pixel_to_8bit_buff(uint16_t x, uint16_t y, uint16_t color)
{
  ((uint8 *)CurrBuffer)[x * _height + y] = Graphics::color565_to_gray8(color);
}

void IRAM_ATTR Buffering::clear_buff()
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

void IRAM_ATTR Buffering::rect_to_16bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
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

void IRAM_ATTR Buffering::rect_to_8bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
{
  uint8_t color_8 = Graphics::color565_to_gray8(color);
  for (uint16_t x = x0; x < x0 + w; x++)
    os_memset((uint8 *)CurrBuffer + x * _height + y0, color_8, h);
}

#include "glcdfont.c"
void IRAM_ATTR Buffering::char_to_16bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size)
{
  uint8_t char_m[6];
  for (uint8_t i = 0; i < 5; i++) // load char to stack
    char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
  char_m[5] = 0;

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

void IRAM_ATTR Buffering::char_to_8bit_buff(uint16_t x0, uint16_t y0, char c, uint16_t color, uint16_t bg, uint8_t size)
{
  uint8_t char_m[6];
  for (uint8_t i = 0; i < 5; i++) // load char to stack
    char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
  char_m[5] = 0;

  uint8_t color_8 = Graphics::color565_to_gray8(color);
  uint8_t bg_8 = Graphics::color565_to_gray8(bg);
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

void IRAM_ATTR Buffering::Bitmap_to_16bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t *colors)
{
  for (uint8_t dw = 0; dw < w; dw++)
    os_memcpy((uint16 *)CurrBuffer + _height * (x0 + dw) + y0, (uint16 *)colors + h * dw, h * 2);
}

void IRAM_ATTR Buffering::Bitmap_to_8bit_buff(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t *colors)
{
  for (uint8_t dw = 0; dw < w; dw++)
    os_memcpy((uint8_t *)CurrBuffer + _height * (x0 + dw) + y0, (uint8_t *)colors + h * dw, h);
}