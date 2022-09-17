#include "Graphics.h"
#include "ST7735.h"
#include "Arduino.h"

#include "stdarg.h"
#include "my_math.h"
#include "Image565.h"
extern  uint8_t _width;
extern  uint8_t _height;

Graphics::Graphics()//: Adafruit_GFX(w, h), connection(TFT_HARD_SPI), -1, -1, _dc(dc)
{
    _freq = 10000000;
};

void Graphics::init() {
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    ST7735_begin(2, _freq);
};

void Graphics::setSPISpeed(uint32_t freq) {
    _freq = freq;

    wait_queue_to_empty();
    setFrequency(_freq); //setFrequency(40 * 1000 * 1000);
};



void Graphics::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    ST7735_Rect_to_queue(x, y, w, h, color);
};
void Graphics::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w && h) {   // Nonzero width and height?
        if (w < 0) {  // If negative width...
            x += w + 1; //   Move X to left edge
            w = -w;     //   Use positive width
        }
        if (x < _width) { // Not off right
            if (h < 0) {    // If negative height...
                y += h + 1;   //   Move Y to top edge
                h = -h;       //   Use positive height
            }
            if (y < _height) { // Not off bottom
                int16_t x2 = x + w - 1;
                if (x2 >= 0) { // Not off left
                    int16_t y2 = y + h - 1;
                    if (y2 >= 0) { // Not off top
                      // Rectangle partly or fully overlaps screen
                        if (x < 0) {
                            x = 0;
                            w = x2 + 1;
                        } // Clip left
                        if (y < 0) {
                            y = 0;
                            h = y2 + 1;
                        } // Clip top
                        if (x2 >= _width) {
                            w = _width - x;
                        } // Clip right
                        if (y2 >= _height) {
                            h = _height - y;
                        } // Clip bottom
                        writeFillRectPreclipped(x, y, w, h, color);
                    }
                }
            }
        }
    }
};
void Graphics::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y + h - 1, w, color);
    writeFastVLine(x, y+1, h-2, color);
    writeFastVLine(x + w-1, y+1, h-2, color);
};

void inline Graphics::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
        if (w < 0) {                        // If negative width...
            x += w + 1;                       //   Move X to left edge
            w = -w;                           //   Use positive width
        }
        if (x < _width) { // Not off right
            int16_t x2 = x + w - 1;
            if (x2 >= 0) { // Not off left
              // Line partly or fully overlaps screen
                if (x < 0) {
                    x = 0;
                    w = x2 + 1;
                } // Clip left
                if (x2 >= _width) {
                    w = _width - x;
                } // Clip right
                writeFillRectPreclipped(x, y, w, 1, color);
            }
        }
    }
};
void inline Graphics::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
        if (h < 0) {                       // If negative height...
            y += h + 1;                      //   Move Y to top edge
            h = -h;                          //   Use positive height
        }
        if (y < _height) { // Not off bottom
            int16_t y2 = y + h - 1;
            if (y2 >= 0) { // Not off top
              // Line partly or fully overlaps screen
                if (y < 0) {
                    y = 0;
                    h = y2 + 1;
                } // Clip top
                if (y2 >= _height) {
                    h = _height - y;
                } // Clip bottom
                writeFillRectPreclipped(x, y, 1, h, color);
            }
        }
    }
};
void Graphics::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    }
    else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            writePixel(y0, x0, color);
        }
        else {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}




void Graphics::writePixel(int16_t x, int16_t y, uint16_t color) {
    ST7735_pixel_to_queue(x, y, color);
   // ST7735_drawPixel(x, y, color);
}

void Graphics::fillScreen(uint16_t color) {
    ST7735_Rect_to_queue(0, 0, _width, _height, color);
}

uint16_t Graphics::color565(uint8_t red, uint8_t green, uint8_t blue) {
    //return  (MAP(red, 0, 255, 0, 31) << 11)|(MAP(green, 0, 255, 0, 63) << 5)|(MAP(blue, 0, 255, 0, 31));//slower
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);  //faster
}

void Graphics::drawImage565(int16_t x, int16_t y, Image565* image) {
    //ST7735_buff_to_queue(x, y, width, height, ram_ptr);
    int16_t w, h;
    int16_t x0 = 0, y0 = 0;

    ((x + image->width) > _width) ? (w = _width - x) : (w = image->width);
    if (x < 0) { w += x; }
    else { x0 = x; }
    ((y + image->height) > _height) ? (h = _height - y) : (h = image->height);
    if (y < 0) { h += y; }
    else { y0 = y; }//

    if (((y + image->height) > _height)) {
        if (x >= 0) x = 0;
        for (uint8_t i = 0; i < w; i++)
            ST7735_buff_to_queue(x0 + i, y0, 1, h, image->ram_ptr + image->height * i - image->height * x);
    }
    else if ((y < 0)) {
        if (x >= 0) x = 0;
        for (uint8_t i = 0; i < w; i++)
            ST7735_buff_to_queue(x0 + i, y0, 1, h, image->ram_ptr + image->height * i - image->height * x - y);
    }
    else
    {
        ST7735_buff_to_queue(x0, y0, w, image->height, image->ram_ptr - image->height * x);
    }

}



void Graphics::invertDisplay(bool i) {
    ST7735_invertDisplay(i);
}

void Graphics::setRotation(uint8_t m) {
    ST7735_setRotation(m);
}

void Graphics::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x < 0) ||              
        (y < 0) ||             
        ((x + 6 * size - 1) >= _width) || 
        ((y + 8 * size - 1) >= _height))   
        return;
    ST7735_char_to_queue(x, y, c, color, bg, size);
}
void Graphics::write(uint8_t c) {
    switch (c) {
    case'\n':
        cursor_x = 0;               // Reset x to zero,
        cursor_y += textsize * 8; // advance y one line
        if (cursor_y + textsize * 8 > _height) cursor_y = 0;
        break;

    case'\t':
        //cursor_x += textsize * 6 * 4;
        if(cursor_x!=0)
            cursor_x = ((cursor_x-1) / (textsize * 4 * 6) + 1)*(textsize * 4 * 6);
        else
            cursor_x += textsize * 4 * 6;

        if (cursor_x + textsize * 6 > _width) {
            cursor_x = 0;
            cursor_y += textsize * 8;
            if (cursor_y + textsize * 8 > _height) cursor_y = 0;
        }
        break;

    case'\r':
        cursor_y += textsize * 8; // advance y one line
        if (cursor_y + textsize * 8 > _height) cursor_y = 0;
        break;
    case' ':
        ST7735_Rect_to_queue(cursor_x, cursor_y, 6 * textsize, 8 * textsize, textbgcolor);
        cursor_x += textsize * 6;
        if (cursor_x + textsize * 6 > _width) {
            cursor_x = 0;
            cursor_y += textsize * 8;
            if (cursor_y + textsize * 8 > _height) 
                cursor_y = 0;
        }
        break;

    default:
        ST7735_char_to_queue(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
        cursor_x += textsize * 6; 
        if (cursor_x + textsize * 6 > _width) {
            cursor_x = 0;                                      
            cursor_y += textsize * 8; 
            if (cursor_y + textsize * 8 > _height) cursor_y = 0;
        }
        break;

    }

}

void Graphics::print(const char* str) {
    for (uint8_t i = 0; i < strlen(str); i++) {
        write(str[i]);
    }
}


void Graphics::printS( int32_t num, uint8_t amount ) {// выравнивание по левому краю
    uint8_t counter = 0;// количество недостающих символов до amount
    if (num < 0) { num = -num; 
        write('-');
    }
    else counter++;

    int32_t divv = 1;
    char str[amount];

    str[0] = num % 10;
    for (uint8_t i = 1; i < amount; i++){
        divv *= 10;
        str[i] = (num / divv) % 10;
    }

    uint8_t flag = 1;
    for (int8_t i = amount-1; i > 0; i--) { 
        
        if (  flag && (str[i] == 0)) {
            counter++; 
        } else {
            flag = 0;
            write(48+str[i]);
            
        }
    }write(48 + str[0]);
   // if(counter!=0) ST7735_Rect_to_queue(cursor_x, cursor_y, 6*textsize*counter, 8*textsize, RED);
    //cursor_x += 6*textsize*counter;
    for (uint8_t i = 0; i < counter; i++) {
        ST7735_char_to_queue(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
        cursor_x += textsize*6;
        if (cursor_x + textsize * 6 > _width) {
            break;
        }
    }
}
void Graphics::printU(uint32_t num, uint8_t amount) {// выравнивание по левому краю
    uint8_t counter = 0;// количество недостающих символов до amount

    int32_t divv = 1;
    char str[amount];

    str[0] = num % 10;
    for (uint8_t i = 1; i < amount; i++) {
        divv *= 10;
        str[i] = (num / divv) % 10;
    }

    uint8_t flag = 1;
    for (int8_t i = amount - 1; i > 0; i--) {

        if (flag && (str[i] == 0)) {
            counter++;
        }
        else {
            flag = 0;
            write(48 + str[i]);

        }
    }write(48 + str[0]);
    // if(counter!=0) ST7735_Rect_to_queue(cursor_x, cursor_y, 6*textsize*counter, 8*textsize, RED);
     //cursor_x += 6*textsize*counter;
    for (uint8_t i = 0; i < counter; i++) {
        ST7735_char_to_queue(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
        cursor_x += textsize * 6;
        if (cursor_x + textsize * 6 > _width) {
            break;
        }
    }
}

void Graphics::printFix(int32_t num, uint8_t amount) {
    int32_t divv = 1;
    for (uint8_t i = 0; i < amount - 1; i++)
        divv *= 10;
    for (uint8_t i = 0; i < amount; i++) {
        write(48 + (num / divv) % 10);
        divv /= 10;
    }
}

void Graphics::print(float numm, uint8_t integer, uint8_t fractional) {////////////
    uint8_t counter = 0;// количество недостающих символов до amount
    if (numm < 0) {
        numm = -numm;
        write('-');
    } else counter++;

    int32_t divv = 1;
    char str[integer];
    uint32_t num = (uint32_t)numm;
    str[0] = num % 10;
    for (uint8_t i = 1; i < integer; i++) {
        divv *= 10;
        str[i] = (num / divv) % 10;
    }

    uint8_t flag = 1;
    for (int8_t i = integer - 1; i > 0; i--) {
        if (flag && (str[i] == 0)) {
            counter++;
        }
        else {
            flag = 0;
            write(48 + str[i]);

        }
    }write(48 + str[0]);

    write('.');
    numm = numm - (float)num;
    
    divv = 1;
    for (uint8_t i = 0; i < fractional-1; i++)  divv *= 10;
    numm *= 10 * divv;
    num = (uint32_t)(numm);
    for (uint8_t i = 0; i < fractional; i++) {
        if (divv == 0) 
            write(48);
        else
            write(48 + (num / divv) % 10);  
        divv /= 10;
    }

    for (uint8_t i = 0; i < counter; i++) {
        ST7735_char_to_queue(cursor_x, cursor_y, ' ', textcolor, textbgcolor, textsize);
        cursor_x += textsize * 6;
        if (cursor_x + textsize * 6 > _width) {
            break;
        }
    }
}

void Graphics::printHex(uint32_t num, uint8_t amount) {
    char str[amount] = {48};
    for (uint8_t i = 0; i < amount; i++) {
        uint8_t h = (num >> (4 * i)) & 0xF;
        str[i] = (h < 10) ? (48 + h) : (65 + h - 10);
    }
    for (int8_t i = amount-1; i >= 0; i--) {
        write(str[i]);
    }
}

void Graphics::printBin(uint32_t num, uint8_t amount) {
    char str[amount] = { 48 };
    str[0] = 48 + (num & 1);

    for (uint8_t i = 1; i < amount; i++) {
        num >>= 1;
        str[i] = 48 + (num & 1);
    }
    for (int8_t i = amount - 1; i >= 0; i--) {
        write(str[i]);
    }
}



void Graphics::printR(int32_t num, uint8_t amount) {// выравнивание по правому краю
    uint8_t counter = 0;// количество недостающих символов до amount
    uint8_t negative = 0;
    if (num < 0) {
        num = -num;
        negative = 1;
    }
    else counter++;

    int32_t divv = 1;
    char str[amount];

    str[0] = num % 10;
    for (uint8_t i = 1; i < amount; i++) {
        divv *= 10;
        str[i] = (num / divv) % 10;
    }

    for (int8_t i = amount - 1; i > 0; i--) {
        if (str[i] == 0) {
            counter++;
        } else 
            break; 

    }
    
    if(counter != 0) ST7735_Rect_to_queue(cursor_x, cursor_y, 6*textsize*counter, 8*textsize, textbgcolor);
    cursor_x += 6*textsize*counter;

    if (negative) write('-');

    for (int8_t i = amount-(counter+negative-1)-1; i >= 0; i--) {
        write(48 + str[i]);
        //Serial.printf("%c",48+str[i]);
    }//Serial.printf("\n");


}

inline static bool is_num(char c) {
    return (c >= '0' && c <= '9');
}
static uint8_t str_to_num(char* str) {
    uint8_t result = 0;
    uint8_t i = 0;
    while (is_num(str[i])){
        result *= 10;
        result += (str[i] - '0');
        i++;    
    } 
    return result;
}

void Graphics::printf(const char* format, ...) {
    va_list factor;         //указатель va_list
    va_start(factor, format);    // устанавливаем указатель
    for (uint8_t i = 0; i < strlen(format); i++)
    {
        if (format[i] == '%') {
            i++;
            if (format[i]!='%') {


                char str[5] = { '\0' };
                uint8_t len = 0;
                uint8_t int_offset = 0;
                while ( is_num(format[i]) || format[i]=='.') {
                    str[len] = format[i]; len++;
                    if(format[i] == '.') int_offset =i;
                    i++;
                    //if (i == sizeof(format)) break;
                }
                

                bool su_flag = 0;
                su:
                switch (format[i]) {

                //case 's':
                //    goto su;
                //    i++;
                //    break;

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
                    //else printS(data,amount);
                    if (amount == 0) { // неизвестно количество знаков 
                        if (su_flag) printU(data);// if unsigned
                        else printS(data);
                    } else {
                        if(su_flag) printU(data, amount);// if unsigned
                        else printS(data, amount);
                    }
                }
                break;

                case 'x':
                {
                    uint32_t data = va_arg(factor, uint32_t);
                    uint8_t amount = str_to_num(str);
                    if (amount == 0)  // неизвестно количество знаков 
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
                    char* str1 = str + int_offset;//
                    fractional = str_to_num(str1);
                    /*Serial.printf("%d %d\n", integer, fractional);
                    Serial.printf("-- %s  %s\n", str, str1);*/
                    print(data, integer, fractional);
                }    
                break;

                case 'b':
                {
                    uint32_t data = va_arg(factor, uint32_t);
                    uint8_t amount = str_to_num(str);
                    if (amount == 0)  // неизвестно количество знаков 
                        printBin(data);
                    else
                        printBin(data, amount);
                }
                break;
                case 's':
                {
                    
                    char* data = va_arg(factor, char*);
                    uint8_t i = 0;
                    while (data[i] != '\0')
                    {
                        write(data[i]);
                        i++;
                    }
                }
                break;
                }
            } else write('%');     
        } else write(format[i]);

        
    }
    va_end(factor); // завершаем обработку параметров

}
