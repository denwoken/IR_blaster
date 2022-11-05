
#pragma once
#include "Arduino.h"

#include "Image565.h"

#ifndef BLACK
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
  {                         \
    int16_t t = a;          \
    a = b;                  \
    b = t;                  \
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
  Graphics() { _freq = 10000000; };
  void init();

  void setSPISpeed(uint32_t freq);
  void invertDisplay(bool i);
  void setRotation(uint8_t m);

  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
  void writePixel(int16_t x, int16_t y, uint16_t color);
  void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  void drawBitmapPreclipped(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color, uint8_t in_flash);
  void drawImagePreclipped(uint16_t x, uint16_t y, Image565 *image);
  void drawImagePreclipped(uint16_t x, uint16_t y, Image_dataset *image);

  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillScreen(uint16_t color);

  void drawImage565(int16_t x, int16_t y, Image565 *image);

  void write(uint8_t c);
  void setTextSize(uint8_t s) { textsize = s; }
  void setCursor(int16_t x, int16_t y)
  {
    cursor_x = x;
    cursor_y = y;
  }

  void setTextbgColor(uint16_t c) { textbgcolor = c; }
  void setTextColor(uint16_t c) { textcolor = c; }
  void setTextColor(uint16_t c, uint16_t bg)
  {
    textcolor = c;
    textbgcolor = bg;
  }

  // print string
  void print(const char *str);

  // Left aligned num
  void printS(int32_t num, uint8_t amount); // signed variable | amount-count of characters
  void printS(int32_t num) { printS(num, 10); }
  void printU(uint32_t num, uint8_t amount); // unsigned variable
  void printU(uint32_t num) { printU(num, 10); }

  // Right aligned num
  void printR(int32_t num, uint8_t amount);
  void printR(int32_t num) { printR(num, 10); }

  //  print float
  void print(float num, uint8_t integer, uint8_t fractional);
  void print(float num) { print(num, 3, 3); }

  // draw with zeroes (fixed length)| printFix(123,6) -> *123000  (vs normal with spaces *123___ )
  void printFix(int32_t num, uint8_t amount);

  // obviously
  void printHex(uint32_t num, uint8_t amount);
  void printHex(uint32_t num) { printHex(num, 8); }
  void printBin(uint32_t num, uint8_t amount);
  void printBin(uint32_t num) { printBin(num, 8); }

  // format print
  // "%%"    ->  %
  // "%5ud"  ->  XXXXX  (unsigned)
  // "%5d"   ->  _XXXXX  (signed)( _ sign)
  // "%2.3f" ->  _xx.xxx  (signed)
  // "%8x"   ->  XXXXXXXX  (hex)
  // "%8b"   ->  XXXXXXXX  (binary)
  // "%s"    ->  ****  (string)

  // uint8_t a = 126
  // printf("0b%8b", a); -> 0b01111110
  void printf(const char *format, ...);

  static uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
  static uint8_t color565_to_gray8(uint16_t color);
  static uint16_t gray8_to_color565(uint8_t gray);

  void Renderer();
  void Clear();

  void CriticalEror(const char *str);

  uint16_t rotation;

protected:
  uint16_t textcolor;   ///< 16-bit background color for print()
  uint16_t textbgcolor; ///< 16-bit text color for print()

  int16_t cursor_x; ///< x location to start print()ing text
  int16_t cursor_y; ///< y location to start print()ing text
  uint8_t textsize; ///< Desired magnification in X-axis of text to print()

  uint32_t _freq = 10000000;
};
extern Graphics tft;
extern uint8_t _width;
extern uint8_t _height;