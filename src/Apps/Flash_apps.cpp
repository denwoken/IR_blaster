#include "Apps/Flash_apps.h"
#include "Arduino.h"
extern "C"
{
#include "eagle_soc.h"
#include "osapi.h"
#include "spi_flash.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "Timer.h"

#include "display_drivers/driver_ST7735.h"
#include "Graphics.h"
extern Graphics tft;
extern uint8_t _width;
extern uint8_t _height;

#include "Inputs.h"
#include "Buttons.h"
extern Button *L_but;
extern Button *R_but;
extern Button *B_but;
extern Button *C_but;
extern Button *T_but;

#include "Menue/menueItem.h"

#include "FlashOptions.h"
extern Global_options Gl_options;

const char *flash_mapps[] = {
    "4M_MAP_256_256",
    "2M",
    "8M_MAP_512_512",
    "16M_MAP_512_512",
    "32M_MAP_512_512",
    "16M_MAP_1024_1024",
    "32M_MAP_1024_1024",
    "64M_MAP_1024_1024",
    "128M_MAP_1024_1024"};

void FlashInfo_app(menueItem *item, void *)
{
  char **str = const_cast<char **>(flash_mapps);
  while (1)
  {
    tft.Clear();
    forcedQuickUpdate();
    item->renderTitle();

    tft.setTextSize(1);
    tft.setTextColor(GREEN, BLACK);
    tft.setCursor(0, 25);
    tft.printf("Flash size =\n%s\n", str[system_get_flash_size_map()]);
    for (uint8_t i = 0; i < 4; i++)
    {
      tft.setCursor(0, 57 + i * 20);
      tft.printf("0x%3x", 0 + 256 * i);
      tft.setCursor(_width - 5 * 6, 57 + i * 20);
      tft.printf("0x%3x", 255 + 256 * i);
      // tft.writeRect(0, 65 + i * 20, 128, 10, WHITE);
      tft.writeFastHLine(0, 65 + i * 20, 128, WHITE);
      tft.writeFastHLine(0, 74 + i * 20, 128, WHITE);
    }

    if (L_but->isClick())
      break;

    uint32_t ptr_data[1024] = {0};
    for (uint8_t i = 0; i < 4; i++)
    {
      soft_updates();
      for (uint16_t j = 0; j < 256; j += 2)
      {

        ets_intr_lock();
        spi_flash_read(((i * 0x100) | (j)) * 0x1000, (uint32_t *)ptr_data, 4096);
        ets_intr_unlock();

        uint32_t count = 0;
        for (uint16_t p = 0; p < 1024; p++)
          if ((ptr_data[p] != 0xffffffff) && (ptr_data[p] != 0))
            count++;

        ets_intr_lock();
        spi_flash_read(((i * 0x100) | (j + 1)) * 0x1000, (uint32_t *)ptr_data, 4096);
        ets_intr_unlock();

        for (uint16_t p = 0; p < 1024; p++)
          if ((ptr_data[p] != 0xffffffff) && (ptr_data[p] != 0))
            count++;

        if (count)
          tft.writeFillRectPreclipped(j / 2, 66 + i * 20, 1, 8, RED);
        else
          tft.writeFillRectPreclipped(j / 2, 66 + i * 20, 1, 8, GREEN);
      }
      system_soft_wdt_feed();
    }
    tft.Renderer();
  }
}
