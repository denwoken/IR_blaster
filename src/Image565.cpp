
#include "Image565.h"
#include "Arduino.h"
#include "Arduino.h"
//#include "menueItem.h"
#include "Pictures.h"

extern "C"
{
#include "eagle_soc.h"
    //#include "gpio.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "display_drivers/driver_ST7735.h"
#include "Graphics.h"
//#include "Timer.h"
extern uint8_t _width;
extern uint8_t _height;

Image565::Image565(uint16_t *ptr, uint8_t w, uint8_t h)
{
    width = w;
    height = h;
    cash_ptr = ptr;
    num_pixels = width * height;
}

Image565::~Image565()
{
    FreeRAM();
}

// void Image565::SetDataPtr(uint16_t* Ptr) {

//}

bool Image565::AllocRAM()
{
    ram_ptr = (uint16_t *)os_malloc(2 * num_pixels);
    if (ram_ptr == NULL)
        return 1;
    else
        return 0;
}
void Image565::LoadfromCASH()
{
    if (ram_ptr != NULL)
    {

        os_memcpy(ram_ptr, cash_ptr, 2 * num_pixels);
    }
}
void Image565::FreeRAM()
{
    if (ram_ptr != NULL)
    {
        os_free(ram_ptr);
        ram_ptr = NULL;
    }
}

void Image565::BrightnessInc()
{
    Color565 c;
    // wait_queue_to_empty();
    for (uint16_t i = 0; i < num_pixels; i++)
    {
        c.RGB = ram_ptr[i];
        (c.R + 1 < 31) ? (c.R += 1) : (c.R = 31);
        (c.G + 2 < 63) ? (c.G += 2) : (c.G = 63);
        (c.B + 1 < 31) ? (c.B += 1) : (c.B = 31);
        ram_ptr[i] = c.RGB;
    }
}
void Image565::BrightnessDec()
{
    BrightnessDec(1);
}

void Image565::BrightnessDec(uint8_t br)
{
    Color565 c;
    // wait_queue_to_empty();
    if (br > 31)
        br = 31; // br = Constrain(br, 0, 31);
    for (uint16_t i = 0; i < num_pixels; i++)
    {
        c.RGB = ram_ptr[i];
        (c.R - br > 0) ? (c.R -= br) : (c.R = 0);
        (c.G - br * 2 > 0) ? (c.G -= br * 2) : (c.G = 0);
        (c.B - br > 0) ? (c.B -= br) : (c.B = 0);
        ram_ptr[i] = c.RGB;
    }
}

// void Image565::wait_end_Picture_draw() {

//}
