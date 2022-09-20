#include "buffering.h"

#include "ST7735.h"
extern uint8_t _height;
extern uint8_t _width;

extern "C"
{
#include <ets_sys.h>
#include "user_interface.h"
#include <osapi.h>
}

#include "FlashOptions.h"
extern Global_options Gl_options;

uint16_t *buff_ptr_0 = NULL;
uint16_t *buff_ptr_1 = NULL;

void init_single_buffer()
{
	buff_ptr_0 = (uint16_t *)os_malloc(128 * 160);
}

void free_single_buffer()
{
	if (buff_ptr_0 != NULL)
		os_free(buff_ptr_0);
}

void SwapBuffers()
{
}

void SendBufferr()
{
}

uint16_t *CurrBuffer()
{
	return buff_ptr_0;
}

void pixel_to_buff(int16_t x, int16_t y, int16_t color)
{
	CurrBuffer()[y * 128 + x] = color;
}

void rect_to_buff(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t color)
{
	for (uint16_t j = y0 * 128; j < (y0 + h) * 128; j += 128)
	{
		for (uint16_t i = x0; i < x0 + w; i++)
		{
			CurrBuffer()[j + i] = color;
		}
	}
	// for (uint16_t j = y0; j < y0 + h; j++)
	// 	for (uint16_t i = x0; i < x0 + w; i++)
	// 		CurrBuffer()[j*128 + i] = color;
}