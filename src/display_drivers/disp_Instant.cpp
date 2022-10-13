

#include "Arduino.h"

#include "display_drivers/disp_Instant.h"
#include "display_drivers/driver_ST7735.h"
#include "display_drivers/SPI.h"

#include "glcdfont.c"

using namespace ST7735;

void ST7735::draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    setAddrWindow(x, y, 1, 1);
    setDataBits(16);
    *(ADDR_16BIT_SPI1W0) = color; // SPI1W0 = color;
    SPI1CMD |= SPIBUSY;
}

void ST7735::draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t lenn = w * h;
    setAddrWindow(x, y, w, h);
    if (lenn > 32)
    {
        setDataBits(32 * 16);

        for (uint16_t i = 0; i < 32; i += 2)
            *(ADDR_16BIT_SPI1W0 + i) = color;

        for (; lenn >= 32; lenn -= 32)
        {
            SPI1CMD |= SPIBUSY;
            WAIT_END_PACK_TRANSFER();
        }
    }
    if (lenn > 0)
    {
        setDataBits(lenn * 16);
        for (uint16_t i = 0; i < lenn; i += 2)
            *(ADDR_16BIT_SPI1W0 + i) = color;
        SPI1CMD |= SPIBUSY;
    }
}

void ST7735::draw_buff(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *colors)
{
    uint16_t lenn = w * h;
    setAddrWindow(x, y, w, h);

    uint16_t pixel_num = 0;
    uint16_t count_last_pix = 0;
    if (lenn > 32)
    {
        setDataBits(512);
        for (; pixel_num + 32 <= lenn; pixel_num += 32)
        {
            for (uint16_t i = 0; i < 16; i++)
            {
                const uint16_t *offset = colors + 2 * i + pixel_num;
                *(&SPI1W0 + i) = (*(offset + 1)) | ((*offset) << 16);
            }
            SPI1CMD |= SPIBUSY;
            WAIT_END_PACK_TRANSFER();
        }
        count_last_pix = lenn - pixel_num;
    }
    if (count_last_pix)
    {
        setDataBits(count_last_pix * 16);
        for (uint16_t i = 0; i < count_last_pix; i++)
        {
            const uint16_t *offset = colors + 2 * i + pixel_num;
            *(&SPI1W0 + i) = (*(offset + 1)) | ((*offset) << 16);
        }
    }
}

void ST7735::draw_char(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size)
{
    setAddrWindow(x, y, 6, 8);
    uint8_t char_m[6];
    for (uint8_t i = 0; i < 5; i++)
        char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
    char_m[5] = 0;

    setDataBits(512);
    uint32_t data_32;
    for (uint16_t i = 0; i < 16; i++)
    {
        uint16_t i_32 = (i << 1);

        if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            data_32 = color << 16;
        else
            data_32 = bcolor << 16;

        i_32++;

        if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            data_32 |= color;
        else
            data_32 |= bcolor;
        *(&SPI1W0 + i) = data_32;
    }
    SPI1CMD |= SPIBUSY;
    WAIT_END_PACK_TRANSFER();

    setDataBits(256);
    for (uint16_t i = 0; i < 8; i++)
    {
        uint16_t i_32 = 32 + (i << 1);

        if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            data_32 = color << 16;
        else
            data_32 = bcolor << 16;

        i_32++;

        if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            data_32 |= color;
        else
            data_32 |= bcolor;
        *(&SPI1W0 + i) = data_32;
    }
    SPI1CMD |= SPIBUSY;
    // WAIT_END_PACK_TRANSFER();
}