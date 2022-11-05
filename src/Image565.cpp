
#include "Image565.h"
#include "Arduino.h"
#include "Arduino.h"

#include "Image_dataset.h"

extern "C"
{
#include "eagle_soc.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "display_drivers/disp_buffering.h"
#include "display_drivers/driver_ST7735.h"
#include "Graphics.h"

#include "FlashOptions.h"

#include <umm_malloc/umm_heap_select.h>
Image565::Image565(Image_dataset &dataset)
{
    SetFlashDataset(dataset);
}

void Image565::SetFlashDataset(Image_dataset &dataset)
{
    width = dataset.width;
    height = dataset.height;
    Flash_ptr = dataset.data_Ptr;
    num_pixels = width * height;
}

// void Image565::SetDataPtr(uint16_t* Ptr) {

//}

bool Image565::AllocDRAM()
{
    if (DataInRam)
        return 1;
    if (Gl_options.buffering == 1)
        Ram_ptr = (uint16_t *)os_malloc(1 * num_pixels);
    else if (Gl_options.buffering == 2)
        Ram_ptr = (uint16_t *)os_malloc(2 * num_pixels);

    if (Ram_ptr == NULL)
        return AllocIRAM();
    else
    {
        DataInRam = true;
        return 0;
    }
}

bool Image565::AllocIRAM()
{
    if (DataInRam)
        return 1;

    HeapSelectIram ephemeral;

    if (Gl_options.buffering == 1)
        Ram_ptr = (uint16_t *)os_malloc(1 * num_pixels);
    else if (Gl_options.buffering == 2)
        Ram_ptr = (uint16_t *)os_malloc(2 * num_pixels);

    if (Ram_ptr == NULL)
        return 1;
    else
    {
        DataInRam = true;
        return 0;
    }
}
void Image565::LoadToRAM()
{
    if (Gl_options.buffering == 1)
        if (Ram_ptr != NULL)
            os_memcpy(Ram_ptr, Flash_ptr, 2 * num_pixels);

    if (Gl_options.buffering == 2)
    {
        for (uint8_t x = 0; x < width; x++)
        {
            uint16_t mass[height];
            os_memcpy(mass, Flash_ptr + height * x, 2 * height);

            uint8_t res[height];
            for (uint8_t y = 0; y < height; y++)
                res[y] = Graphics::color565_to_gray8(mass[y]);

            os_memcpy((uint8_t *)Ram_ptr + height * x, res, height);
        }
    }
}
void Image565::FreeRAM()
{
    if (Ram_ptr != NULL)
    {
        DataInRam = false;
        os_free(Ram_ptr);
        Ram_ptr = NULL;
    }
}
u16 Image565::getPixel(u16 x, u16 y)
{
    uint16_t index = x * _height + y;
    if (DataInRam)
        return Ram_ptr[index];
    else
    {
        uint16_t color;
        if (Gl_options.buffering == 1)
            os_memcpy(&color, (uint8_t *)Flash_ptr + index, 1);
        else if (Gl_options.buffering == 2)
            os_memcpy(&color, (uint8_t *)Flash_ptr + index, 2);
        return color;
    }
    // Color565 c;
    // c.RGB = Ram_ptr[index];
    // (c.R + 1 < 31) ? (c.R += 1) : (c.R = 31);
    // (c.G + 2 < 63) ? (c.G += 2) : (c.G = 63);
    // (c.B + 1 < 31) ? (c.B += 1) : (c.B = 31);
    // return c.RGB;
}

// void Image565::Brightness(uint8_t b)
// {
//     brightness = b >> 3;
// }

// void Image565::BrightnessInc(uint8_t b)
// {
//     if (brightness + b < MAX_BRIGHTNESS)
//         brightness += b;
//     else
//         brightness = MAX_BRIGHTNESS;
// }
// void Image565::BrightnessDec(uint8_t b)
// {
//     if (brightness - b > 0)
//         brightness -= b;
//     else
//         brightness = 0;
// }

// void Image565::BrightnessInc()
// {
//     if (brightness < MAX_BRIGHTNESS)
//         brightness++;
// }
// void Image565::BrightnessDec()
// {
//     if (brightness > 0)
//         brightness--;
// }
/*
void Image565::BrightnessInc()
{
    Color565 c;
    for (uint16_t i = 0; i < num_pixels; i++)
    {
        c.RGB = Ram_ptr[i];
        (c.R + 1 < 31) ? (c.R += 1) : (c.R = 31);
        (c.G + 2 < 63) ? (c.G += 2) : (c.G = 63);
        (c.B + 1 < 31) ? (c.B += 1) : (c.B = 31);
        Ram_ptr[i] = c.RGB;
    }
}

void Image565::BrightnessDec(uint8_t br)
{
    Color565 c;
    if (br > 31)
        br = 31; // br = Constrain(br, 0, 31);
    for (uint16_t i = 0; i < num_pixels; i++)
    {
        c.RGB = Ram_ptr[i];
        (c.R - br > 0) ? (c.R -= br) : (c.R = 0);
        (c.G - br * 2 > 0) ? (c.G -= br * 2) : (c.G = 0);
        (c.B - br > 0) ? (c.B -= br) : (c.B = 0);
        Ram_ptr[i] = c.RGB;
    }
}
*/
// void Image565::wait_end_Picture_draw() {

//}
