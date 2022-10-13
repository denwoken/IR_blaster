

#include "Arduino.h"

#include "glcdfont.c"

extern "C"
{
#include <osapi.h>
}

#include "display_drivers/driver_ST7735.h"
#include "display_drivers/SPI.h"
#include "display_drivers/disp_buffering.h"
#include "display_drivers/disp_Queue.h"

uint8_t _height = ST7735_TFTHEIGHT_160;
uint8_t _width = ST7735_TFTWIDTH_128;

#include "FlashOptions.h"

using namespace ST7735;

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

void ST7735::displayInit(const uint8_t *addr)
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
        sendCommand(cmd, addr, numArgs);
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

static inline void SPI_DC_HIGH(void) { GPOS = 1 << 2; }; //_dc
static inline void SPI_DC_LOW(void) { GPOC = 1 << 2; }   //_dc

void ST7735::begin(int8_t dc, uint32_t freq)
{
    pinMode(dc, OUTPUT);    //_dc
    digitalWrite(dc, HIGH); //_dc

    begin_spi(freq);

    displayInit(Rcmd1);
    displayInit(Rcmd2red);
    displayInit(Gl_options.GammaPOS);
    displayInit(Gl_options.GammaNEG);
    displayInit(Rcmd3);

    setRotation(0);
    invertDisplay(0);
}

void ST7735::sendCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_end_sending();        // wait for the queue to empty && stop buffer sending
    writeCommand(commandByte); // Send the command byte
    for (int i = 0; i < numDataBytes; i++)
    {
        write(*dataBytes); // Send the data bytes
        dataBytes++;
    }
}
void ST7735::sendCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_end_sending();        // wait for the queue to empty && stop buffer sending
    writeCommand(commandByte); // Send the command byte
    for (int i = 0; i < numDataBytes; i++)
        write(pgm_read_byte(dataBytes++));
}
void ST7735::sendCommand16(uint16_t commandWord, const uint8_t *dataBytes, uint8_t numDataBytes)
{
    wait_end_sending(); // wait for the queue to empty && stop buffer sending
    if (numDataBytes == 0)
        writeCommand16(commandWord); // Send the command word

    for (int i = 0; i < numDataBytes; i++)
    {
        writeCommand16(commandWord); // Send the command word
        commandWord++;
        write16((uint16_t)pgm_read_byte(dataBytes++));
    }
}
void ST7735::writeCommand(uint8_t cmd)
{
    // wait_queue_to_empty(); // wait for the queue to empty
    // wait_end_sending(); // wait for the queue to empty && stop buffer sending
    WAIT_END_PACK_TRANSFER();
    SPI_DC_LOW();
    write(cmd);
    WAIT_END_PACK_TRANSFER();
    SPI_DC_HIGH();
}

void ST7735::writeCommand16(uint16_t cmd)
{
    // wait_queue_to_empty(); // wait for the queue to empty
    // wait_end_sending(); // wait for the queue to empty && stop buffer sending
    WAIT_END_PACK_TRANSFER();
    SPI_DC_LOW();
    write(cmd);
    WAIT_END_PACK_TRANSFER();
    SPI_DC_HIGH();
}

void ST7735::invertDisplay(bool i)
{
    wait_end_sending(); // wait for the queue to empty && stop buffer sending
    writeCommand(i ? ST77XX_INVON : ST77XX_INVOFF);
}

void ST7735::setRotation(uint8_t m)
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
    sendCommand(ST77XX_MADCTL, &madctl, 1);
}

void ST7735::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    writeCommand(ST77XX_CASET); // Column addr set
    write32(ST7735_setAddr(y, h));
    writeCommand(ST77XX_RASET); // Row addr set
    write32(ST7735_setAddr(x, w));
    writeCommand(ST77XX_RAMWR); // write to RAM
}
