#include "Graphics.h"

#include "Arduino.h"

#include "display_drivers/disp_Queue.h"
#include "display_drivers/driver_ST7735.h"
#include "display_drivers/disp_buffering.h"
#include "display_drivers/disp_Instant.h"
#include "display_drivers/SPI.h"

#include "stdarg.h"
#include "my_math.h"
#include "Image565.h"

extern uint8_t _width;
extern uint8_t _height;

#include "FlashOptions.h"
extern Global_options Gl_options;

void Graphics::init()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  ST7735::begin(2, _freq);
};

void Graphics::setSPISpeed(uint32_t freq)
{
  _freq = freq;

  wait_queue_to_empty();
  setFrequency(_freq); // setFrequency(40 * 1000 * 1000);
};

void Graphics::invertDisplay(bool i)
{
  ST7735::invertDisplay(i);
}

void Graphics::setRotation(uint8_t m)
{
  rotation = m;
  ST7735::setRotation(rotation);
}

//####
//####
//####
//###################################################################################################################

void Graphics::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
  if ((x < 0) ||
      (y < 0) ||
      ((x + 6 * size - 1) >= _width) ||
      ((y + 8 * size - 1) >= _height))
    return;

  switch (Gl_options.buffering)
  {
  case 0:
    if (Gl_options.spi_queue)
      disp_Queue::char_to_queue(x, y, c, color, bg, size);
    else
      ST7735::draw_char(x, y, c, color, bg, size);
    break;
  case 1:
    Buffering::char_to_16bit_buff(x, y, c, color, bg, size);
    break;
  case 2:
    Buffering::char_to_8bit_buff(x, y, c, color, bg, size);
    break;
  }
}

void Graphics::writePixel(int16_t x, int16_t y, uint16_t color)
{
  switch (Gl_options.buffering)
  {
  case 0:
    if (Gl_options.spi_queue)
      disp_Queue::pixel_to_queue(x, y, color);
    else
      ST7735::draw_pixel(x, y, color);
    break;
  case 1:
    Buffering::pixel_to_16bit_buff(x, y, color);
    break;
  case 2:
    Buffering::pixel_to_8bit_buff(x, y, color);
    break;
  }
}

void Graphics::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  switch (Gl_options.buffering)
  {
  case 0:
    if (Gl_options.spi_queue)
      disp_Queue::Rect_to_queue(x, y, w, h, color);
    else
      ST7735::draw_rect(x, y, w, h, color);
    break;
  case 1:
    Buffering::rect_to_16bit_buff(x, y, w, h, color);
    break;
  case 2:
    Buffering::rect_to_8bit_buff(x, y, w, h, color);
    break;
  }
};

void Graphics::drawRGBBitmapPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color)
{
  switch (Gl_options.buffering)
  {
  case 0:
    if (Gl_options.spi_queue)
      disp_Queue::buff_to_queue(x, y, w, h, color);
    else
      ST7735::draw_buff(x, y, w, h, color);
    break;
  case 1:
    // Buffering::
    break;
  case 2:

    break;
  }
}

//####
//####
//####
//###################################################################################################################
void Graphics::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  writeLine(x0, y0, x1, y1, color);
  writeLine(x1, y1, x2, y2, color);
  writeLine(x2, y2, x0, y0, color);
}
void Graphics::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{

  int16_t a, b, y, last;

  if (y0 > y1)
  {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }
  if (y1 > y2)
  {
    _swap_int16_t(y2, y1);
    _swap_int16_t(x2, x1);
  }
  if (y0 > y1)
  {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }

  if (y0 == y2)
  { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    writeFastHLine(a, y0, b - a + 1, color);
    return;
  }

  int16_t dx01 = x1 - x0,
          dy01 = y1 - y0,
          dx02 = x2 - x0,
          dy02 = y2 - y0,
          dx12 = x2 - x1,
          dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++)
  {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    writeFastHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++)
  {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    writeFastHLine(a, y, b - a + 1, color);
  }
}

void inline Graphics::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  if ((y >= 0) && (y < _height) && w)
  { // Y on screen, nonzero width
    if (w < 0)
    {             // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width)
    { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0)
      { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0)
        {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width)
        {
          w = _width - x;
        } // Clip right
        writeFillRectPreclipped(x, y, w, 1, color);
      }
    }
  }
};
void inline Graphics::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  if ((x >= 0) && (x < _width) && h)
  { // X on screen, nonzero height
    if (h < 0)
    {             // If negative height...
      y += h + 1; //   Move Y to top edge
      h = -h;     //   Use positive height
    }
    if (y < _height)
    { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0)
      { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0)
        {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height)
        {
          h = _height - y;
        } // Clip bottom
        writeFillRectPreclipped(x, y, 1, h, color);
      }
    }
  }
};
void Graphics::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      writePixel(y0, x0, color);
    }
    else
    {
      writePixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void Graphics::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  if (w && h)
  { // Nonzero width and height?
    if (w < 0)
    {             // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    // Serial.printf("%d\t%d\n", _width, _height);
    if (x < _width)
    { // Not off right
      if (h < 0)
      {             // If negative height...
        y += h + 1; //   Move Y to top edge
        h = -h;     //   Use positive height
      }
      if (y < _height)
      { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0)
        { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0)
          { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0)
            {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0)
            {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width)
            {
              w = _width - x;
            } // Clip right
            if (y2 >= _height)
            {
              h = _height - y;
            } // Clip bottom
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
};
void Graphics::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  writeFastHLine(x, y, w, color);
  writeFastHLine(x, y + h - 1, w, color);
  writeFastVLine(x, y + 1, h - 2, color);
  writeFastVLine(x + w - 1, y + 1, h - 2, color);
};

void Graphics::fillScreen(uint16_t color)
{

  if (Gl_options.buffering)
  {
    if (!color)
      Buffering::clear_buff();
    else
      writeFillRectPreclipped(0, 0, _width, _height, color);
  }
  else
    writeFillRectPreclipped(0, 0, _width, _height, color);
}

void Graphics::drawImage565(int16_t x, int16_t y, Image565 *image)
{
  // ST7735_buff_to_queue(x, y, width, height, ram_ptr);
  int16_t w, h;
  int16_t x0 = 0, y0 = 0;

  ((x + image->width) > _width) ? (w = _width - x) : (w = image->width);
  if (x < 0)
    w += x;
  else
    x0 = x;
  ((y + image->height) > _height) ? (h = _height - y) : (h = image->height);
  if (y < 0)
    h += y;
  else
    y0 = y;

  if (((y + image->height) > _height))
  {
    if (x >= 0)
      x = 0;
    for (uint8_t i = 0; i < w; i++)
      drawRGBBitmapPreclipped(x0 + i, y0, 1, h, image->ram_ptr + image->height * i - image->height * x);
  }
  else if ((y < 0))
  {
    if (x >= 0)
      x = 0;
    for (uint8_t i = 0; i < w; i++)
      drawRGBBitmapPreclipped(x0 + i, y0, 1, h, image->ram_ptr + image->height * i - image->height * x - y);
  }
  else
  {
    drawRGBBitmapPreclipped(x0, y0, w, image->height, image->ram_ptr - image->height * x);
  }
}

//####
//####
//####
//###################################################################################################################
uint16_t Graphics::color565(uint8_t red, uint8_t green, uint8_t blue)
{
  // return  (MAP(red, 0, 255, 0, 31) << 11)|(MAP(green, 0, 255, 0, 63) << 5)|(MAP(blue, 0, 255, 0, 31));//slower
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); // faster
}

uint8_t Graphics::color565_to_gray8(uint16_t c)
{
  Color565 color;
  color.RGB = c;
  return (((color.R << 1) + color.G + (color.B << 1)) / 3) * 4;
};

uint16_t Graphics::gray8_to_color565(uint8_t gray)
{
  Color565 color;
  color.R = gray >> 3;
  color.G = gray >> 2;
  color.B = gray >> 3;
  return color.RGB;
};
//####
//####
//####
//###################################################################################################################
void Graphics::write(uint8_t c)
{
  switch (c)
  {
  case '\n':
    cursor_x = 0;             // Reset x to zero,
    cursor_y += textsize * 8; // advance y one line
    if (cursor_y + textsize * 8 > _height)
      cursor_y = 0;
    break;

  case '\t':
    // cursor_x += textsize * 6 * 4;
    if (cursor_x != 0)
      cursor_x = ((cursor_x - 1) / (textsize * 4 * 6) + 1) * (textsize * 4 * 6);
    else
      cursor_x += textsize * 4 * 6;

    if (cursor_x + textsize * 6 > _width)
    {
      cursor_x = 0;
      cursor_y += textsize * 8;
      if (cursor_y + textsize * 8 > _height)
        cursor_y = 0;
    }
    break;

  case '\r':
    cursor_y += textsize * 8; // advance y one line
    if (cursor_y + textsize * 8 > _height)
      cursor_y = 0;
    break;
  case ' ':
    writeFillRectPreclipped(cursor_x, cursor_y, 6 * textsize, 8 * textsize, textbgcolor);
    cursor_x += textsize * 6;
    if (cursor_x + textsize * 6 > _width)
    {
      cursor_x = 0;
      cursor_y += textsize * 8;
      if (cursor_y + textsize * 8 > _height)
        cursor_y = 0;
    }
    break;

  default:
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize * 6;
    if (cursor_x + textsize * 6 > _width)
    {
      cursor_x = 0;
      cursor_y += textsize * 8;
      if (cursor_y + textsize * 8 > _height)
        cursor_y = 0;
    }
    break;
  }
}

void Graphics::print(const char *str)
{
  for (uint8_t i = 0; i < strlen(str); i++)
    write(str[i]);
}

void Graphics::printS(int32_t num, uint8_t amount)
{                      // ������������ �� ������ ����
  uint8_t counter = 0; // ���������� ����������� �������� �� amount
  if (num < 0)
  {
    num = -num;
    write('-');
  }
  else
    counter++;

  int32_t divv = 1;
  char str[amount];

  str[0] = num % 10;
  for (uint8_t i = 1; i < amount; i++)
  {
    divv *= 10;
    str[i] = (num / divv) % 10;
  }

  uint8_t flag = 1;
  for (int8_t i = amount - 1; i > 0; i--)
  {

    if (flag && (str[i] == 0))
      counter++;
    else
    {
      flag = 0;
      write(48 + str[i]);
    }
  }
  write(48 + str[0]);
  // if(counter!=0) writeFillRectPreclipped(cursor_x, cursor_y, 6*textsize*counter, 8*textsize, RED);
  // cursor_x += 6*textsize*counter;
  for (uint8_t i = 0; i < counter; i++)
  {
    drawChar(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
    cursor_x += textsize * 6;
    if (cursor_x + textsize * 6 > _width)
      break;
  }
}
void Graphics::printU(uint32_t num, uint8_t amount)
{                      // ������������ �� ������ ����
  uint8_t counter = 0; // ���������� ����������� �������� �� amount

  int32_t divv = 1;
  char str[amount];

  str[0] = num % 10;
  for (uint8_t i = 1; i < amount; i++)
  {
    divv *= 10;
    str[i] = (num / divv) % 10;
  }

  uint8_t flag = 1;
  for (int8_t i = amount - 1; i > 0; i--)
  {

    if (flag && (str[i] == 0))
      counter++;
    else
    {
      flag = 0;
      write(48 + str[i]);
    }
  }
  write(48 + str[0]);
  // if(counter!=0) writeFillRectPreclipped(cursor_x, cursor_y, 6*textsize*counter, 8*textsize, RED);
  // cursor_x += 6*textsize*counter;
  for (uint8_t i = 0; i < counter; i++)
  {
    drawChar(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
    cursor_x += textsize * 6;
    if (cursor_x + textsize * 6 > _width)
      break;
  }
}

void Graphics::printFix(int32_t num, uint8_t amount)
{
  int32_t divv = 1;
  for (uint8_t i = 0; i < amount - 1; i++)
    divv *= 10;
  for (uint8_t i = 0; i < amount; i++)
  {
    write(48 + (num / divv) % 10);
    divv /= 10;
  }
}

void Graphics::print(float numm, uint8_t integer, uint8_t fractional)
{                      ////////////
  uint8_t counter = 0; // ���������� ����������� �������� �� amount
  if (numm < 0)
  {
    numm = -numm;
    write('-');
  }
  else
    counter++;

  int32_t divv = 1;
  char str[integer];
  uint32_t num = (uint32_t)numm;
  str[0] = num % 10;
  for (uint8_t i = 1; i < integer; i++)
  {
    divv *= 10;
    str[i] = (num / divv) % 10;
  }

  uint8_t flag = 1;
  for (int8_t i = integer - 1; i > 0; i--)
  {
    if (flag && (str[i] == 0))
      counter++;
    else
    {
      flag = 0;
      write(48 + str[i]);
    }
  }
  write(48 + str[0]);

  write('.');
  numm = numm - (float)num;

  divv = 1;
  for (uint8_t i = 0; i < fractional - 1; i++)
    divv *= 10;
  numm *= 10 * divv;
  num = (uint32_t)(numm);
  for (uint8_t i = 0; i < fractional; i++)
  {
    if (divv == 0)
      write(48);
    else
      write(48 + (num / divv) % 10);
    divv /= 10;
  }

  for (uint8_t i = 0; i < counter; i++)
  {
    drawChar(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
    cursor_x += textsize * 6;
    if (cursor_x + textsize * 6 > _width)
      break;
  }
}

void Graphics::printHex(uint32_t num, uint8_t amount)
{
  char str[amount] = {48};
  for (uint8_t i = 0; i < amount; i++)
  {
    uint8_t h = (num >> (4 * i)) & 0xF;
    str[i] = (h < 10) ? (48 + h) : (65 + h - 10);
  }
  for (int8_t i = amount - 1; i >= 0; i--)
    write(str[i]);
}

void Graphics::printBin(uint32_t num, uint8_t amount)
{
  char str[amount] = {48};
  str[0] = 48 + (num & 1);

  for (uint8_t i = 1; i < amount; i++)
  {
    num >>= 1;
    str[i] = 48 + (num & 1);
  }
  for (int8_t i = amount - 1; i >= 0; i--)
    write(str[i]);
}

void Graphics::printR(int32_t num, uint8_t amount)
{                      // ������������ �� ������� ����
  uint8_t counter = 0; // ���������� ����������� �������� �� amount
  uint8_t negative = 0;
  if (num < 0)
  {
    num = -num;
    negative = 1;
  }
  else
    counter++;

  int32_t divv = 1;
  char str[amount];

  str[0] = num % 10;
  for (uint8_t i = 1; i < amount; i++)
  {
    divv *= 10;
    str[i] = (num / divv) % 10;
  }

  for (int8_t i = amount - 1; i > 0; i--)
  {
    if (str[i] == 0)
      counter++;
    else
      break;
  }

  if (counter != 0)
    writeFillRectPreclipped(cursor_x, cursor_y, 6 * textsize * counter, 8 * textsize, textbgcolor);
  cursor_x += 6 * textsize * counter;

  if (negative)
    write('-');

  for (int8_t i = amount - (counter + negative - 1) - 1; i >= 0; i--)
    write(48 + str[i]);
  // Serial.printf("%c",48+str[i]);
  // Serial.printf("\n");
}

inline static bool is_num(char c)
{
  return (c >= '0' && c <= '9');
}
static uint8_t str_to_num(char *str)
{
  uint8_t result = 0;
  uint8_t i = 0;
  while (is_num(str[i]))
  {
    result *= 10;
    result += (str[i] - '0');
    i++;
  }
  return result;
}

void Graphics::printf(const char *format, ...)
{
  va_list factor;           //��������� va_list
  va_start(factor, format); // ������������� ���������
  for (uint8_t i = 0; i < strlen(format); i++)
  {
    if (format[i] == '%')
    {
      i++;
      if (format[i] != '%')
      {

        char str[5] = {'\0'};
        uint8_t len = 0;
        uint8_t int_offset = 0;
        while (is_num(format[i]) || format[i] == '.')
        {
          str[len] = format[i];
          len++;
          if (format[i] == '.')
            int_offset = i;
          i++;
          // if (i == sizeof(format)) break;
        }

        bool su_flag = 0;
      su:
        switch (format[i])
        {

          // case 's':
          //     goto su;
          //     i++;
          //     break;

        case 'u':
          su_flag = 1;
          i++;
          goto su;
          break;

        case 'd':
        {
          int data = va_arg(factor, int);
          uint8_t amount = str_to_num(str);
          // if (amount == 0) printS(data);
          // else printS(data,amount);
          if (amount == 0)
          { // ���������� ���������� ������
            if (su_flag)
              printU(data); // if unsigned
            else
              printS(data);
          }
          else
          {
            if (su_flag)
              printU(data, amount); // if unsigned
            else
              printS(data, amount);
          }
        }
        break;

        case 'x':
        {
          uint32_t data = va_arg(factor, uint32_t);
          uint8_t amount = str_to_num(str);
          if (amount == 0) // ���������� ���������� ������
            printHex(data);
          else
            printHex(data, amount);
        }
        break;

        case 'f':
        {

          uint8_t integer, fractional;
          double data = va_arg(factor, double);
          integer = str_to_num(str);
          char *str1 = str + int_offset - 1; // Mistake!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          if (*str1 == '.')                  // Mistake!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            str1++;
          fractional = str_to_num(str1);
          /* Serial.printf("%d %d\n", integer, fractional);
          Serial.printf("--|%s|%s|\n", str, str1);*/
          print(data, integer, fractional);
        }
        break;

        case 'b':
        {
          uint32_t data = va_arg(factor, uint32_t);
          uint8_t amount = str_to_num(str);
          if (amount == 0)
            printBin(data);
          else
            printBin(data, amount);
        }
        break;
        case 's':
        {

          char *data = va_arg(factor, char *);
          uint8_t i = 0;
          while (data[i] != '\0')
          {
            write(data[i]);
            i++;
          }
        }
        break;
        }
      }
      else
        write('%');
    }
    else
      write(format[i]);
  }
  va_end(factor);
}

void Graphics::Renderer()
{
  switch (Gl_options.buffering)
  {
  case 0:
    break;

  case 1:
    Buffering::Send_single_Bufferr();
    break;
  case 2:
    Buffering::SwapBuffers();
    Buffering::Start_sending_double_Bufferr();
    break;
  }
}
void Graphics::Clear()
{
  Buffering::clear_buff();
}

void Graphics::CriticalEror(const char *string)
{

  char str[strlen_P(string)];
  strcpy_P(str, string);

  wait_end_sending();
  uint8_t a = Gl_options.buffering;
  Gl_options.buffering = 0;
  uint8_t b = Gl_options.spi_queue;
  Gl_options.spi_queue = 0;

  fillScreen(BLUE);
  setTextSize(1);
  setTextColor(WHITE, BLUE);
  setCursor(0, 10);
  printf(str); // ST7735::draw_rect(0, 0, _width, _height, BLUE);

  Gl_options.buffering = a;
  Gl_options.spi_queue = b;

  system_restart();
  while (1)
  {
  }
}