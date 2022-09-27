
#pragma once

#include "Arduino.h"
#include "ST7735.h"
//#include <ets_sys.h>
#include "Image565.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
    {                       \
        int16_t t = a;      \
        a = b;              \
        b = t;              \
    }
#endif

typedef union Color565
{
    struct
    {
        unsigned int R : 5;
        unsigned int G : 6;
        unsigned int B : 5;
    };
    uint16_t RGB;
} Color565;

class Graphics //: public Print
{
public:
    Graphics();
    void init();
    void setSPISpeed(uint32_t freq);

    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

    void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    // void writeLine(Vector vec, uint16_t color) { writeLine(vec.x0, vec.y0, vec.x1, vec.y1, color); }
    /**/
    void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    void fillScreen(uint16_t color);

    void drawImage565(int16_t x, int16_t y, Image565 *image);

    void writePixel(int16_t x, int16_t y, uint16_t color);

    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);

    void write(uint8_t c);
    void setTextSize(uint8_t s) { textsize = s; }
    void setCursor(int16_t x, int16_t y)
    {
        cursor_x = x;
        cursor_y = y;
    }

    void setTextColor(uint16_t c) { textcolor = c; }
    void setTextbgColor(uint16_t c) { textbgcolor = c; }
    void setTextColor(uint16_t c, uint16_t bg)
    {
        textcolor = c;
        textbgcolor = bg;
    }

    void print(const char *str);

    void printS(int32_t num, uint8_t amount);
    void printS(int32_t num) { printS(num, 10); }
    void printU(uint32_t num, uint8_t amount);
    void printU(uint32_t num) { printU(num, 10); }

    void printR(int32_t num, uint8_t amount);
    void printR(int32_t num) { printR(num, 10); }

    void print(float num, uint8_t integer, uint8_t fractional);
    void print(float num) { print(num, 3, 3); }

    void printFix(int32_t num, uint8_t amount);

    void printHex(uint32_t num, uint8_t amount);
    void printHex(uint32_t num) { printHex(num, 8); }
    void printBin(uint32_t num, uint8_t amount);
    void printBin(uint32_t num) { printBin(num, 8); }
    void printf(const char *format, ...);

    /*


    void drawRGBBitmapPreclipped(int16_t x, int16_t y, uint16_t* pcolors, int16_t w, int16_t h);
*/

    void invertDisplay(bool i);
    void setRotation(uint8_t m);

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

    void Renderer();
    void Clear();

    uint16_t rotation;

protected:
    uint16_t textcolor;   ///< 16-bit background color for print()
    uint16_t textbgcolor; ///< 16-bit text color for print()

    int16_t cursor_x; ///< x location to start print()ing text
    int16_t cursor_y; ///< y location to start print()ing text
    uint8_t textsize; ///< Desired magnification in X-axis of text to print()

    uint32_t _freq = 10000000;
};
