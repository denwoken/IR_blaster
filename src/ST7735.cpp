
#include "Arduino.h"
#include "ST7735.h"
uint8_t _height = 160;
uint8_t _width = 128;
#include "glcdfont.c"
#include <osapi.h>

#include "FlashOptions.h"
extern Global_options Gl_options;
#include "buffering.h"

static void IRAM_ATTR intr_spi(void);

static inline void SPI_DC_HIGH(void) { GPOS = 1 << 2; } //_dc
static inline void SPI_DC_LOW(void) { GPOC = 1 << 2; }  //_dc

static const uint8_t PROGMEM
    Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
        15,                           // 15 commands in list:
        ST77XX_SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
        150,                          //     150 ms delay
        ST77XX_SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                          //     500 ms delay
        ST7735_FRMCTR1, 3,            //  3: Framerate ctrl - normal mode, 3 arg:
        0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3,            //  4: Framerate ctrl - idle mode, 3 args:
        0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6,            //  5: Framerate - partial mode, 6 args:
        0x01, 0x2C, 0x2D,             //     Dot inversion mode
        0x01, 0x2C, 0x2D,             //     Line inversion mode
        ST7735_INVCTR, 1,             //  6: Display inversion ctrl, 1 arg:
        0x07,                         //     No inversion
        ST7735_PWCTR1, 3,             //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,             //     -4.6V
        0x84,             //     AUTO mode
        ST7735_PWCTR2, 1, //  8: Power control, 1 arg, no delay:
        0xC5,             //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
        ST7735_PWCTR3, 2, //  9: Power control, 2 args, no delay:
        0x0A,             //     Opamp current small
        0x00,             //     Boost frequency
        ST7735_PWCTR4, 2, // 10: Power control, 2 args, no delay:
        0x8A,             //     BCLK/2,
        0x2A,             //     opamp current small & medium low
        ST7735_PWCTR5, 2, // 11: Power control, 2 args, no delay:
        0x8A, 0xEE,
        ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
        0x0E,
        ST77XX_INVOFF, 0, // 13: Don't invert display, no args
        ST77XX_MADCTL, 1, // 14: Mem access ctl (directions), 1 arg:
        0xC8,             //     row/col addr, bottom-top refresh
        ST77XX_COLMOD, 1, // 15: set color mode, 1 arg, no delay:
        0x05

}, //     16-bit color

    Rcmd2red[] = {       // 7735R init, part 2 (red tab only)
        2,               //  2 commands in list:
        ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F,      //     XEND = 127
        ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x9F},     //     XEND = 159

    Rcmd3[] = {                        // 7735R init, part 3 (red or green tab)
        2, ST77XX_NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
        10,                            //     10 ms delay
        ST77XX_DISPON, ST_CMD_DELAY,   //  4: Main screen turn on, no args w/delay
        10};                           //     100 ms delay

typedef struct spi_command
{
    uint8_t id = 0;

    uint8_t x = 0;
    uint8_t y = 0;

    uint8_t w = 0;
    uint8_t h = 0;

    uint32_t xa = 0;
    uint32_t ya = 0;

    uint16_t len = 0;
    uint8_t char_id = 0;
    union
    {
        struct
        {
            uint16_t value0;
            uint16_t value1;
        };
        uint16_t *ptr;
    } color;
} spi_command;

spi_command spi_queue[BUFF_SIZE + 1];

void ST7735_displayInit(const uint8_t *addr)
{

    uint8_t numCommands, cmd, numArgs;
    uint16_t ms;

    numCommands = pgm_read_byte(addr++); // Number of commands to follow

    while (numCommands--)
    {                                    // For each command...
        cmd = pgm_read_byte(addr++);     // Read command
        numArgs = pgm_read_byte(addr++); // Number of args to follow
        ms = numArgs & ST_CMD_DELAY;     // If hibit set, delay follows args
        numArgs &= ~ST_CMD_DELAY;        // Mask out delay bit
        ST7735_sendCommand(cmd, addr, numArgs);
        addr += numArgs;

        if (ms)
        {
            ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
            if (ms == 255)
                ms = 500; // If 255, delay for 500 ms
            delay(ms);
        }
    }
}

// void ST7735_setfreq(uint32_t freq)
// {
// }

void ST7735_begin_spi_intr()
{
    begin_spi_intr(intr_spi);
}

void ST7735_begin(int8_t dc, uint32_t freq)
{
    pinMode(dc, OUTPUT);
    digitalWrite(dc, HIGH);
    begin_spi(freq);
    ST7735_begin_spi_intr();
    ST7735_displayInit(Rcmd1);
    ST7735_displayInit(Rcmd2red);
    ST7735_displayInit(Gl_options.GammaPOS);
    ST7735_displayInit(Gl_options.GammaNEG);
    ST7735_displayInit(Rcmd3);

    ST7735_setRotation(0);
    ST7735_invertDisplay(0);
}

void ST7735_sendCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_queue_to_empty();     // while (queue_is_Nempty()) { asm volatile("NOP\n"); }; // wait for the queue to empty
    writeCommand(commandByte); // Send the command byte
    for (int i = 0; i < numDataBytes; i++)
    {
        write(*dataBytes); // Send the data bytes
        dataBytes++;
    }
}
void ST7735_sendCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_queue_to_empty();     // while (queue_is_Nempty()) { asm volatile("NOP\n"); }; // wait for the queue to empty
    writeCommand(commandByte); // Send the command byte
    for (int i = 0; i < numDataBytes; i++)
    {
        write(pgm_read_byte(dataBytes++));
    }
}
void ST7735_sendCommand16(uint16_t commandWord, const uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_queue_to_empty(); // while (queue_is_Nempty()) { asm volatile("NOP\n"); }; // wait for the queue to empty
    if (numDataBytes == 0)
    {
        writeCommand16(commandWord); // Send the command word
    }
    for (int i = 0; i < numDataBytes; i++)
    {
        writeCommand16(commandWord); // Send the command word
        commandWord++;
        write16((uint16_t)pgm_read_byte(dataBytes++));
    }
}
void writeCommand(uint8_t cmd)
{
    while (SPI1CMD & SPIBUSY)
    {
        asm volatile("NOP\n");
    };
    SPI_DC_LOW();
    write(cmd);
    while (SPI1CMD & SPIBUSY)
    {
        asm volatile("NOP\n");
    };
    SPI_DC_HIGH();
}
void writeCommand16(uint16_t cmd)
{
    while (SPI1CMD & SPIBUSY)
    {
        asm volatile("NOP\n");
    };
    SPI_DC_LOW();
    write(cmd);
    while (SPI1CMD & SPIBUSY)
    {
        asm volatile("NOP\n");
    };
    SPI_DC_HIGH();
}

void ST7735_invertDisplay(bool i)
{
    wait_queue_to_empty(); // while (queue_is_Nempty()) { asm volatile("NOP\n"); }; // wait for the queue to empty
    writeCommand(i ? ST77XX_INVON : ST77XX_INVOFF);
}

void ST7735_setRotation(uint8_t m)
{
    uint8_t madctl = 0;
    switch (m % 4)
    {
    case 0:
        madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
        _height = (ST7735_TFTHEIGHT_160);
        _width = (ST7735_TFTWIDTH_128);
        break;
    case 1:
        madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_RGB;
        _height = (ST7735_TFTWIDTH_128);
        _width = (ST7735_TFTHEIGHT_160);
        break;
    case 2:
        madctl = ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
        _height = (ST7735_TFTHEIGHT_160);
        _width = (ST7735_TFTWIDTH_128);
        break;
    case 3:
        madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
        _height = (ST7735_TFTWIDTH_128);
        _width = (ST7735_TFTHEIGHT_160);
        break;
    }
    // while (queue_is_Nempty()) { asm volatile("NOP\n"); }; // wait for the queue to empty
    ST7735_sendCommand(ST77XX_MADCTL, &madctl, 1);
}

void ST7735_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    writeCommand(ST77XX_CASET); // Column addr set
    write32(ST7735_setAddr(y, h));
    writeCommand(ST77XX_RASET); // Row addr set
    write32(ST7735_setAddr(x, w));
    writeCommand(ST77XX_RAMWR); // write to RAM
}

static bool spi_intr_flag = 0;

void IRAM_ATTR intr_spi(void)
{
    static uint16_t counter_execute = 0;
    static uint32_t pixel_num_buff_tr = 0;
    static bool first_trans_flag = 1;
    static uint16_t pixels_left = 0;
    static uint8_t char_m[6];
    static uint16_t bcolor, color;
    // uint32_t ew = READ_PERI_REG(0x3ff00020);
    // uint32_t we = READ_PERI_REG(SPI_SLAVE(HSPI_));
    WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
    uint8_t *id_command = &spi_queue[counter_execute].id;

    switch (*id_command)
    {
    case NONE:
    {
        Serial.println("FACK IT SHIT!");
        // Serial.printf("%08x\t%08x\n", ew,we);
        // Serial.printf("%08x\t%08x\n", READ_PERI_REG(REG_SPI_BASE(SPI_) + 0xF8), READ_PERI_REG(REG_SPI_BASE(SPI_) + 0xF8));
        /* Serial.println(*id_command);
         Serial.println(spi_intr_flag);
         Serial.println();
         Serial.println(counter_execute);
         Serial.println(counter_add);
         Serial.println();
         Serial.printf("   \t id\t xa\t\t ya\t\t len\t val\n");
         for(int i = 0;i<=BUFF_SIZE;i++)
             Serial.printf("%d)\t %d\t %d\t %d\t %d\t %d\n",i,
                 spi_queue[i].id,
                 spi_queue[i].xa,
                 spi_queue[i].ya,
                 spi_queue[i].len,
                 spi_queue[i].color.value);
        */
        while (1)
        {
            delayMicroseconds(100);
        };
    }
    break;
    case CHAR_COMM_2:
    {

        if (first_trans_flag)
        {
            first_trans_flag = 0;
            pixel_num_buff_tr = 0;

            char c = spi_queue[counter_execute].char_id;
            for (uint8_t i = 0; i < 5; i++)
                char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
            char_m[5] = 0;

            bcolor = spi_queue[counter_execute].color.value1;
            color = spi_queue[counter_execute].color.value0;
            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;

            SPI_INTR_DISABLE();

            writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, 8 * 2));
            writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, 6 * 2));
            writeCommand(ST77XX_RAMWR);

            setDataBits(512);
            WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
            SPI_INTR_ENABLE();
        }

        if (pixel_num_buff_tr < (8 * 6 * 4 / 2 - 16))
        {

            uint16_t data_16 = 0;
            for (uint16_t i = 0; i < 8; i++)
            {
                uint16_t i_32 = pixel_num_buff_tr + i;

                if (1 & (char_m[i_32 >> 4] >> (i_32 & 7)))
                    data_16 = color;
                else
                    data_16 = bcolor;
                *(ADDR_16BIT_SPI1W0 + 2 * i) = data_16;
                //*(ADDR_16BIT_SPI1W0 + 2*i + 1 ) = data_16;
                *(ADDR_16BIT_SPI1W0 + 2 * i + 16) = data_16;
                //*(ADDR_16BIT_SPI1W0 + 2*i + 17) = data_16;
            }

            pixel_num_buff_tr += 16;

            SPI1CMD |= SPIBUSY;
        }
        else
        {
            first_trans_flag = 1;

            uint16_t data_16 = 0;
            for (uint16_t i = 0; i < 8; i++)
            {
                uint16_t i_32 = pixel_num_buff_tr + i;

                if (1 & (char_m[i_32 >> 4] >> (i_32 & 7)))
                    data_16 = color;
                else
                    data_16 = bcolor;

                *(ADDR_16BIT_SPI1W0 + 2 * i) = data_16;
                //*(ADDR_16BIT_SPI1W0 + 2*i + 1 ) = data_16;
                *(ADDR_16BIT_SPI1W0 + 2 * i + 16) = data_16;
                //*(ADDR_16BIT_SPI1W0 + 2*i + 17) = data_16;
            }

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++; //   ��������� �������� ��� counter_execute
            *id_command = 0;
            if (spi_queue[counter_execute].id == 0)
            {
                SPI_INTR_DISABLE();
                spi_intr_flag = 0;
            }
            SPI1CMD |= SPIBUSY;
        }
    }
    break;

    case CHAR_COMM_1:
    {
        if (first_trans_flag)
        {
            first_trans_flag = 0;

            uint8_t c = spi_queue[counter_execute].char_id;
            for (uint8_t i = 0; i < 5; i++)
                char_m[i] = (uint8_t)pgm_read_byte(&font[c * 5 + i]);
            char_m[5] = 0;

            bcolor = spi_queue[counter_execute].color.value1;
            color = spi_queue[counter_execute].color.value0;
            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;

            SPI_INTR_DISABLE();
            writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, 8));
            writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, 6));
            writeCommand(ST77XX_RAMWR);

            WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
            SPI_INTR_ENABLE();

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
        }
        else
        {
            first_trans_flag = 1;

            setDataBits(256);

            uint32_t data_32;
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

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++; //   ��������� �������� ��� counter_execute
            *id_command = 0;
            if (spi_queue[counter_execute].id == NONE)
            {
                SPI_INTR_DISABLE();
                spi_intr_flag = 0;
            }
        }
        SPI1CMD |= SPIBUSY;
    }
    break;

    case FILL_RECT_COMM:
    {
        if (first_trans_flag)
        {
            first_trans_flag = 0;
            pixel_num_buff_tr = 0;

            SPI_INTR_DISABLE();

            // color = spi_queue[counter_execute].color.value0;
            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;
            uint8_t w = spi_queue[counter_execute].w;
            uint8_t h = spi_queue[counter_execute].h;
            pixels_left = w * h;
            writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, h));
            writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, w));
            writeCommand(ST77XX_RAMWR);

            if (pixels_left < 10)
            {
                first_trans_flag = 1;

                setDataBits(16 * pixels_left);
                for (uint16_t i = 0; i < pixels_left; i += 2)
                    *(ADDR_16BIT_SPI1W0 + i) = spi_queue[counter_execute].color.value0;

                SPI1CMD |= SPIBUSY;

                *id_command = 0;

                if (counter_execute == BUFF_SIZE)
                    counter_execute = 0;
                else
                    counter_execute++; // ��������� �������� ��� counter_execute

                while (SPI1CMD & SPIBUSY)
                    asm volatile("NOP\n");

                if (spi_queue[counter_execute].id != 0)
                {
                    WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
                    SPI_INTR_ENABLE();
                }
                else
                    spi_intr_flag = 0;
                break;
            }
            else if (pixels_left > 32)
            {
                setDataBits(512);
                for (uint16_t i = 0; i < 32; i += 2)
                    *(ADDR_16BIT_SPI1W0 + i) = spi_queue[counter_execute].color.value0;
            }

            WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
            SPI_INTR_ENABLE();
        }

        if (pixels_left > 32)
        {
            pixels_left -= 32;
            SPI1CMD |= SPIBUSY;
        }
        else
        {
            first_trans_flag = 1;

            setDataBits(pixels_left * 16);

            for (uint16_t i = 0; i < pixels_left; i += 2)
                *(ADDR_16BIT_SPI1W0 + i) = spi_queue[counter_execute].color.value0;

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++; //   ��������� �������� ��� counter_execute

            *id_command = 0;
            // Serial.printf("c_ex %d\n", counter_execute);
            if (spi_queue[counter_execute].id == 0)
            {
                spi_intr_flag = 0;
                SPI_INTR_DISABLE();
            }
            // else{ WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN); SPI_INTR_ENABLE(); }

            SPI1CMD |= SPIBUSY;
        }
    }
    break;

    case BUFF_COMM:
    {
        static uint16_t *addr_16_data;
        if (first_trans_flag)
        {
            SPI_INTR_DISABLE();

            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;
            uint8_t w = spi_queue[counter_execute].w;
            uint8_t h = spi_queue[counter_execute].h;

            writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, h));
            addr_16_data = spi_queue[counter_execute].color.ptr;
            pixels_left = w * h;
            writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, w));
            pixel_num_buff_tr = 0;
            first_trans_flag = 0;
            writeCommand(ST77XX_RAMWR);

            if (pixels_left > 32)
                setDataBits(512);
            WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);
            SPI_INTR_ENABLE();
        }

        if (pixels_left > 32)
        {
            pixels_left -= 32;
            for (uint16_t i = 0; i < 16; i++)
            {
                const uint16_t *offset = addr_16_data + (i << 1) + pixel_num_buff_tr;
                *(&SPI1W0 + i) = (*(offset + 1)) | ((*(offset)) << 16);
            }

            pixel_num_buff_tr += 32;

            SPI1CMD |= SPIBUSY;
        }
        else
        {
            first_trans_flag = 1;
            setDataBits(pixels_left * 16);

            for (uint16_t i = 0; i < pixels_left / 2; i++)
            {
                const uint16_t *offset = addr_16_data + (i << 1) + pixel_num_buff_tr;
                *(&SPI1W0 + i) = (*(offset + 1)) | ((*(offset)) << 16);
            }
            // if (size % 2 == 1) {                                                     // �������� ���������� ��������!!!!!!!!!!!!!!!!!!!!!
            //     const uint16_t* offset = addr_16_data + size-1 + pixel_num_buff_tr;
            //     *(&SPI1W0 + size/2 - 1) = (*(offset + 1)) | ((*(offset)) << 16);
            // }

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++; //   ��������� �������� ��� counter_execute
            *id_command = 0;

            if (spi_queue[counter_execute].id == 0)
            {
                SPI_INTR_DISABLE();
                spi_intr_flag = 0;
            }

            SPI1CMD |= SPIBUSY;
        }
    }
    break;
    }
}
// Gl_options.spi_queue
static uint16_t counter_add = 0;
void ST7735_pixel_to_queue(int16_t x, int16_t y, uint16_t color)
{
    if (!queue_is_Nempty() || !Gl_options.spi_queue)
    {
        ST7735_setAddrWindow(x, y, 1, 1);
        setDataBits(16);
        *(ADDR_16BIT_SPI1W0) = color; // SPI1W0 = color;
        SPI1CMD |= SPIBUSY;
        return;
    }

    while (spi_queue[counter_add].id != NONE)
        asm volatile("NOP\n"); // yield();

    spi_queue[counter_add].id = FILL_RECT_COMM;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = 1;
    spi_queue[counter_add].h = 1;
    spi_queue[counter_add].color.value0 = color;

    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++; //   ��������� �������� ��� counter_add
}

void ST7735_Rect_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    uint16_t lenn = w * h;
    if (!queue_is_Nempty() || !Gl_options.spi_queue) //
    {

        ST7735_setAddrWindow(x, y, w, h);
        if (lenn <= 4)
        {
            setDataBits(lenn * 16);
            for (uint16_t i = 0; i < lenn; i += 2)
                *(ADDR_16BIT_SPI1W0 + i) = color;
            SPI1CMD |= SPIBUSY;

            return;
        }
        if (lenn > 32)
        {
            setDataBits(32 * 16);

            for (uint16_t i = 0; i < 32; i += 2)
                *(ADDR_16BIT_SPI1W0 + i) = color;

            for (; lenn >= 32; lenn -= 32)
            {
                SPI1CMD |= SPIBUSY;
                while (SPI1CMD & SPIBUSY)
                    asm volatile("NOP\n");
            }
        }

        if (lenn > 0)
        {
            setDataBits(lenn * 16);
            for (uint16_t i = 0; i < lenn; i += 2)
                *(ADDR_16BIT_SPI1W0 + i) = color;
            SPI1CMD |= SPIBUSY;
            while (SPI1CMD & SPIBUSY)
                asm volatile("NOP\n");
        }
        return;
    }

    while (spi_queue[counter_add].id != NONE)
        asm volatile("NOP\n"); // yield();

    spi_queue[counter_add].id = FILL_RECT_COMM;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = w;
    spi_queue[counter_add].h = h;
    spi_queue[counter_add].color.value0 = color;
    spi_queue[counter_add].len = lenn;

    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++;
    //   ��������� �������� ��� counter_add

    if (!queue_is_Nempty())
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
}

void ST7735_buff_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *colors)
{
    uint16_t lenn = w * h;
    if (!Gl_options.spi_queue) //
    {

        ST7735_setAddrWindow(x, y, w, h);

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
                while (SPI1CMD & SPIBUSY)
                    asm volatile("NOP\n");
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

    while (spi_queue[counter_add].id != NONE)
        asm volatile("NOP\n"); // yield();

    spi_queue[counter_add].id = BUFF_COMM;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = w;
    spi_queue[counter_add].h = h;
    spi_queue[counter_add].color.ptr = colors;
    spi_queue[counter_add].len = lenn;

    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++; //   ��������� �������� ��� counter_add

    if (!queue_is_Nempty())
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
}

void wait_end_sending()
{

    wait_queue_to_empty();
    if (spi_intr_flag)
    {
        spi_intr_flag = 0;
        SPI_INTR_DISABLE();
    }
    Buffering::stop_buffer_sending();
}

void ST7735_char_to_queue(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size)
{
    if (!Gl_options.spi_queue) //
    {
        ST7735_setAddrWindow(x, y, 6, 8);

        //(uint8_t)pgm_read_byte(&font[c * 5 + i]);
        uint8_t *char_m = (uint8_t *)os_malloc(5);
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
        while (SPI1CMD & SPIBUSY)
            asm volatile("NOP\n");

        setDataBits(256);

        for (uint16_t i = 0; i < 8; i++)
        {
            uint16_t i_32 = 32 + (i << 1);

            if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            {
                data_32 = color << 16;
            }
            else
            {
                data_32 = bcolor << 16;
            }

            i_32++;

            if (1 & (char_m[i_32 >> 3] >> (i_32 & 7)))
            {
                data_32 |= color;
            }
            else
            {
                data_32 |= bcolor;
            }
            *(&SPI1W0 + i) = data_32;
        }
        SPI1CMD |= SPIBUSY;
        while (SPI1CMD & SPIBUSY)
            asm volatile("NOP\n");
        os_free(char_m);

        return;
    }

    while (spi_queue[counter_add].id != NONE)
    {
        asm volatile("NOP\n");
    }; // yield();

    switch (size)
    {
    case 1:
        spi_queue[counter_add].id = CHAR_COMM_1;
        break;
    case 2:
        spi_queue[counter_add].id = CHAR_COMM_2;
        break;
    }

    uint8_t h = 8 * size;
    uint8_t w = 6 * size;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = w;
    spi_queue[counter_add].h = h;
    spi_queue[counter_add].color.value0 = color;
    spi_queue[counter_add].color.value1 = bcolor;
    spi_queue[counter_add].len = w * h;
    spi_queue[counter_add].char_id = c;
    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++; //   ��������� �������� ��� counter_add

    if (!queue_is_Nempty())
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
}

bool queue_is_Nempty() { return spi_intr_flag; };
void wait_queue_to_empty()
{
    while (spi_intr_flag)
    {
        asm volatile("NOP\nNOP\n");
    }
};