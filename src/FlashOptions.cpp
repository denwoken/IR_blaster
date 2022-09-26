#include "Arduino.h"
extern "C"
{
#include "eagle_soc.h"
#include "spi_flash.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "my_math.h"
#include "Gamma&Tests.h"
#include "ST7735.h"
#include "Graphics.h"
#include "Inputs.h"
#include "Buttons.h"
extern Graphics tft;

#include "buffering.h"

#include "FlashOptions.h"
#define align_32(a) ((a - 1) / 4 + 1) * 4

Global_options Gl_options;
static const uint16_t len = align_32(sizeof(Global_options) + 1);

void set_def_settings()
{
  // default settings

  Gl_options.show_fps = 1;
  Gl_options.show_heap = 1;
  Gl_options.buffering = 0;
  Gl_options.spi_queue = 1;
  Gl_options.CPU_speed = 160;
  Gl_options.SPI_speed = 40; //*1000*1000
  set_def_gamma();
}

void set_def_gamma()
{
  uint8_t GAMCTRP1[]{
      1,                      //  4 commands in list:
      ST7735_GMCTRP1, 16,     //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12, //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d, //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10};

  uint8_t GMCTRN1[]{
      1,
      ST7735_GMCTRN1,
      16, //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03,
      0x1d,
      0x07,
      0x06, //     (Not entirely necessary, but provides
      0x2E,
      0x2C,
      0x29,
      0x2D, //      accurate colors)
      0x2E,
      0x2E,
      0x37,
      0x3F,
      0x00,
      0x00,
      0x02,
      0x10,
  };

  os_memcpy(&Gl_options.GammaPOS, GAMCTRP1, 19);
  os_memcpy(&Gl_options.GammaNEG, GMCTRN1, 19);
}

void draw_settings()
{
  for (uint8_t i = 0; i < sizeof(Global_options); i++)
    Serial.printf("%02x ", ((uint8_t *)&Gl_options)[i]);
  Serial.printf("\n");
}

void init_global_options()
{
  uint8_t flag[4];

  ets_intr_lock();
  spi_flash_read(GLOBAL_OPTIONS_SECTOR * 0x1000, (uint32_t *)flag, 4);
  ets_intr_unlock();

  // Serial.printf("%08x\n", ((uint32_t*)flag)[0] );
  if (flag[0] == 0xff)
  {
    set_def_settings();
    save_settings();
  }
  else
  {
    load_settings();
  }
}

void apply_system_settings()
{

  tft.setSPISpeed(Gl_options.SPI_speed * 1000 * 1000);
  system_update_cpu_freq(Gl_options.CPU_speed);
  if (!Gl_options.spi_queue)
    disable_queue();

  switch (Gl_options.buffering)
  {
  case 0:
    free_all_buffers();
    break;
  case 1:
    disable_queue();
    init_single_buffer();
    break;
  case 2:
    disable_queue();
    init_double_buffer();
    break;
  }
}

void load_settings()
{
  uint8_t *ptr_data = (uint8_t *)os_malloc(len);

  ets_intr_lock();
  spi_flash_read(GLOBAL_OPTIONS_SECTOR * 0x1000, (uint32_t *)ptr_data, len);
  ets_intr_unlock();

  os_memcpy(&Gl_options, ptr_data + 1, sizeof(Global_options));
  os_free(ptr_data);
}

void save_settings()
{
  uint8_t *ptr_data = (uint8_t *)os_malloc(len);

  ptr_data[0] = 0;
  os_memcpy(ptr_data + 1, &Gl_options, sizeof(Global_options));

  ets_intr_lock();
  spi_flash_erase_sector(GLOBAL_OPTIONS_SECTOR);
  spi_flash_write(GLOBAL_OPTIONS_SECTOR * 0x1000, (uint32_t *)ptr_data, len);
  ets_intr_unlock();

  os_free(ptr_data);
}
