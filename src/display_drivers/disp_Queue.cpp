#include "display_drivers/disp_Queue.h"

#include "Arduino.h"

#include "display_drivers/driver_ST7735.h"
#include "display_drivers/disp_buffering.h"
#include "display_drivers/SPI.h"

#include "glcdfont.c"

extern "C"
{
#include <osapi.h>
}
using namespace disp_Queue;
//#include "FlashOptions.h"
//   extern Global_options Gl_options;

static void IRAM_ATTR intr_spi(void);

void disp_Queue::init_display_Queue()
{
    begin_spi_intr(intr_spi);
}

typedef struct spi_command
{
    uint8_t id = 0; // id_ command
    uint8_t x, y = 0;
    uint8_t w, h = 0;
    union
    {
        uint8_t char_id = 0;
        uint8_t free_flag;
    };
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

static bool spi_intr_flag = 0;
static uint16_t counter_add = 0;
static uint16_t counter_execute = 0;
void IRAM_ATTR intr_spi(void)
{
    WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);

    static uint32_t pixel_num_buff_tr = 0;
    static uint8_t first_trans_flag = 1;
    static uint16_t pixels_left = 0;
    static uint8_t char_m[6];
    static uint16_t bcolor, color;

    uint8_t *id_command = &spi_queue[counter_execute].id;

    switch (*id_command)
    {
    case NONE:
    {
        Serial.println("Something went wrong !");
        Serial.println("Critical Queue Erorr");
        // Serial.printf("counter_execute = %d\tcounter_add=%d\nspi_intr_flag=%d\n", counter_execute, counter_add, spi_intr_flag);

        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute - 4, spi_queue[counter_execute - 4].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute - 3, spi_queue[counter_execute - 3].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute - 2, spi_queue[counter_execute - 2].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute - 1, spi_queue[counter_execute - 1].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute, spi_queue[counter_execute].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute + 1, spi_queue[counter_execute + 1].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute + 2, spi_queue[counter_execute + 2].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute + 3, spi_queue[counter_execute + 3].id);
        // Serial.printf("spi_queue[%d].id = %d\n", counter_execute + 4, spi_queue[counter_execute + 4].id);

        // Serial.printf("------------------------------\n");
        // Serial.printf("x=%d|y=%d|w=%d|h=%d|char_id=%d|value0=%d|value1=%d|len=%d|\n",
        //               spi_queue[counter_execute - 1].x,
        //               spi_queue[counter_execute - 1].y,
        //               spi_queue[counter_execute - 1].w,
        //               spi_queue[counter_execute - 1].h,
        //               spi_queue[counter_execute - 1].char_id,
        //               spi_queue[counter_execute - 1].color.value0,
        //               spi_queue[counter_execute - 1].color.value1,
        //               spi_queue[counter_execute - 1].len);
        // Serial.printf("x=%d|y=%d|w=%d|h=%d|char_id=%d|value0=%d|value1=%d|len=%d|\n",
        //               spi_queue[counter_execute].x,
        //               spi_queue[counter_execute].y,
        //               spi_queue[counter_execute].w,
        //               spi_queue[counter_execute].h,
        //               spi_queue[counter_execute].char_id,
        //               spi_queue[counter_execute].color.value0,
        //               spi_queue[counter_execute].color.value1,
        //               spi_queue[counter_execute].len);
        // Serial.printf("x=%d|y=%d|w=%d|h=%d|char_id=%d|value0=%d|value1=%d|len=%d|\n",
        //               spi_queue[counter_execute + 1].x,
        //               spi_queue[counter_execute + 1].y,
        //               spi_queue[counter_execute + 1].w,
        //               spi_queue[counter_execute + 1].h,
        //               spi_queue[counter_execute + 1].char_id,
        //               spi_queue[counter_execute + 1].color.value0,
        //               spi_queue[counter_execute + 1].color.value1,
        //               spi_queue[counter_execute + 1].len);

        while (1)
            delayMicroseconds(100);
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

            ST7735::writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, 8 * 2));
            ST7735::writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, 6 * 2));
            ST7735::writeCommand(ST77XX_RAMWR);

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
                counter_execute++; //

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
            ST7735::writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, 8));
            ST7735::writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, 6));
            ST7735::writeCommand(ST77XX_RAMWR);

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
                counter_execute++;

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

            color = spi_queue[counter_execute].color.value0;
            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;
            uint8_t w = spi_queue[counter_execute].w;
            uint8_t h = spi_queue[counter_execute].h;
            pixels_left = w * h;

            ST7735::writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, h));
            ST7735::writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, w));
            ST7735::writeCommand(ST77XX_RAMWR);

            if (pixels_left < 6)
            {
                first_trans_flag = 1;

                setDataBits(16 * pixels_left);
                for (uint16_t i = 0; i < pixels_left; i += 2)
                    *(ADDR_16BIT_SPI1W0 + i) = color;

                SPI1CMD |= SPIBUSY;

                *id_command = 0;

                if (counter_execute == BUFF_SIZE)
                    counter_execute = 0;
                else
                    counter_execute++; // ��������� �������� ��� counter_execute
                WAIT_END_PACK_TRANSFER();
                if (spi_queue[counter_execute].id != NONE)
                {
                    WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
                    SPI_INTR_ENABLE();
                }
                else
                    spi_intr_flag = 0;

                return;
            }
            else if (pixels_left > 32)
            {

                setDataBits(512);
                for (uint16_t i = 0; i < 32; i += 2)
                    *(ADDR_16BIT_SPI1W0 + i) = color;
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
                *(ADDR_16BIT_SPI1W0 + i) = color;

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++; //   ��������� �������� ��� counter_execute

            *id_command = 0;

            if (spi_queue[counter_execute].id == NONE)
            {
                spi_intr_flag = 0;
                SPI_INTR_DISABLE();
            }

            SPI1CMD |= SPIBUSY;
        }
    }
    break;

    case BUFF_COMM:
    {
        static uint16_t *addr_16_data;

        if (first_trans_flag == 2)
        {
            first_trans_flag = 1;
            if (spi_queue[counter_execute].free_flag)
                os_free(addr_16_data);

            if (counter_execute == BUFF_SIZE)
                counter_execute = 0;
            else
                counter_execute++;
            *id_command = 0;

            if (spi_queue[counter_execute].id == NONE)
            {
                SPI_INTR_DISABLE();
                spi_intr_flag = 0;
            }
            WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
            return;
        }

        if (first_trans_flag)
        {
            SPI_INTR_DISABLE();

            uint8_t x = spi_queue[counter_execute].x;
            uint8_t y = spi_queue[counter_execute].y;
            uint8_t w = spi_queue[counter_execute].w;
            uint8_t h = spi_queue[counter_execute].h;

            ST7735::writeCommand(ST77XX_CASET);
            write32(ST7735_setAddr(y, h));
            addr_16_data = spi_queue[counter_execute].color.ptr;
            pixels_left = w * h;
            ST7735::writeCommand(ST77XX_RASET);
            write32(ST7735_setAddr(x, w));
            pixel_num_buff_tr = 0;
            first_trans_flag = 0;
            ST7735::writeCommand(ST77XX_RAMWR);

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
            first_trans_flag = 2;
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
            SPI1CMD |= SPIBUSY;
        }
    }
    break;
    }
}

void IRAM_ATTR disp_Queue::pixel_to_queue(int16_t x, int16_t y, uint16_t color)
{

    if (!queue_is_Nempty()) // if queue is empty   transfer data without a queue, because sending one pixel take a little time
    {
        ST7735::setAddrWindow(x, y, 1, 1);
        setDataBits(16);
        *(ADDR_16BIT_SPI1W0) = color; // SPI1W0 = color;
        SPI1CMD |= SPIBUSY;
        WAIT_END_PACK_TRANSFER();
        return;
    }

    while (spi_queue[counter_add].id != NONE) // expect for a free place in the queue (queue is overfull)
        asm volatile("NOP\n");

    // load data to the queue

    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = 1;
    spi_queue[counter_add].h = 1;
    spi_queue[counter_add].color.value0 = color;
    spi_queue[counter_add].id = FILL_RECT_COMM;
    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++;
}

void IRAM_ATTR disp_Queue::Rect_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{

    if (!queue_is_Nempty()) // if queue is empty   transfer data without a queue (if it take a little time)
    {
        uint16_t lenn = w * h;
        if (lenn <= 4) // sending 1-4 pixel take a little time ( transfer data without a queue)
        {
            ST7735::setAddrWindow(x, y, w, h);
            setDataBits(lenn * 16);
            for (uint16_t i = 0; i < lenn; i += 2)
                *(ADDR_16BIT_SPI1W0 + i) = color;
            SPI1CMD |= SPIBUSY;
            // WAIT_END_PACK_TRANSFER();
            return;
        }
    }

    while (spi_queue[counter_add].id != NONE) // expect for a free place in the queue (queue is overfull)
        asm volatile("NOP\n");

    // load data to the queue
    SPI_INTR_DISABLE();
    spi_queue[counter_add].id = FILL_RECT_COMM;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = w;
    spi_queue[counter_add].h = h;
    spi_queue[counter_add].color.value0 = color;
    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++;

    if (!queue_is_Nempty()) // if queue was empty, change queue state and start interrupt
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
    else
        SPI_INTR_ENABLE();
}

void IRAM_ATTR disp_Queue::Bitmap_to_queue(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *colors, uint8_t free_flag)
{
    while (spi_queue[counter_add].id != NONE) // expect for a free place in the queue (queue is overfull)
        asm volatile("NOP\n");

    // load data to the queue
    SPI_INTR_DISABLE();
    spi_queue[counter_add].id = BUFF_COMM;
    spi_queue[counter_add].x = x;
    spi_queue[counter_add].y = y;
    spi_queue[counter_add].w = w;
    spi_queue[counter_add].h = h;
    spi_queue[counter_add].color.ptr = colors;
    spi_queue[counter_add].free_flag = free_flag;

    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++;

    if (!queue_is_Nempty()) // if queue was empty, change queue state and start interrupt
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
    else
        SPI_INTR_ENABLE();
}

void IRAM_ATTR disp_Queue::char_to_queue(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bcolor, uint8_t size)
{
    while (spi_queue[counter_add].id != NONE) // expect for a free place in the queue (queue is overfull)
        asm volatile("NOP\n");

    SPI_INTR_DISABLE();
    // load data to the queue
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
    spi_queue[counter_add].char_id = c;

    if (counter_add == BUFF_SIZE)
        counter_add = 0;
    else
        counter_add++;

    if (!queue_is_Nempty()) // if queue was empty, change queue state and start interrupt
    {
        WRITE_PERI_REG(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN | SPI_TRANS_DONE);
        spi_intr_flag = 1;
        SPI_INTR_ENABLE();
    }
    else
        SPI_INTR_ENABLE();
}

void IRAM_ATTR wait_end_sending()
{
    wait_queue_to_empty();
    Buffering::wait_end_buffer_sending();
}

bool IRAM_ATTR queue_is_Nempty() { return spi_intr_flag; };

void IRAM_ATTR wait_queue_to_empty()
{
    while (spi_intr_flag)
        asm volatile("NOP\nNOP\n");
};