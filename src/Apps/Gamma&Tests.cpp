#include "Arduino.h"
extern "C"
{
#include "eagle_soc.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "my_math.h"
#include "Apps/Gamma&Tests.h"

#include "display_drivers/disp_Queue.h"
#include "display_drivers/driver_ST7735.h"
#include "display_drivers/disp_Instant.h"
#include "Graphics.h"

#include "Inputs.h"
#include "Buttons.h"

#include "Menue/menueItem.h"

#include "Timer.h"

#include "FlashOptions.h"

/*
uint8_t GAMCTRP1[]{
  1,                              //  4 commands in list:
  ST7735_GMCTRP1, 16        ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
  0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
  0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
  0x29, 0x25, 0x2B, 0x39,
  0x00, 0x01, 0x03, 0x10
};

uint8_t GMCTRN1[]{
  1,
  ST7735_GMCTRN1, 16    ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
  0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
  0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
  0x2E, 0x2E, 0x37, 0x3F,
  0x00, 0x00, 0x02, 0x10,
};
*/

void Gamma_app(menueItem *item, void *ptr)
{
  tft.fillScreen(0);
  forcedQuickUpdate();

  uint8_t *GAMCTRP1 = (uint8_t *)(&Gl_options.GammaPOS);
  uint8_t *GMCTRN1 = (uint8_t *)(&Gl_options.GammaNEG);

  const char *reg_names_pos[] = {
      "VRFP   ",
      "VOS0P  ",
      "PKP0   ",
      "PKP1   ",
      "PKP2   ",
      "PKP3   ",
      "PKP4   ",
      "PKP5   ",
      "PKP6   ",
      "PKP7   ",
      "PKP8   ",
      "PKP9   ",
      "SELV0P ",
      "SELV1P ",
      "SELV62P",
      "SELV63P",
  };

  const char *reg_names_neg[] = {
      "VRF0N  ",
      "VOS0N  ",
      "PKN0   ",
      "PKN1   ",
      "PKN2   ",
      "PKN3   ",
      "PKN4   ",
      "PKN5   ",
      "PKN6   ",
      "PKN7   ",
      "PKN8   ",
      "PKN9   ",
      "SELV0N ",
      "SELV1N ",
      "SELV62N",
      "SELV63N",
  };

  bool chosen_GAMCTRP1 = 0;
  uint8_t counter = 0;

  L_but->setIncfreq(20); // ������� ���������� Hz
  R_but->setIncfreq(20);
  B_but->setIncfreq(25);
  T_but->setIncfreq(25);
  tft.setRotation(3);

  while (1)
  {
    for (uint32_t j = 0; j <= 31; j++)
    {
      tft.writeFillRect(160 - 16 * 4, j * 4, 16, 4, j);
      tft.writeFillRect(160 - 16 * 3, j * 4, 16, 4, j << 6);
      tft.writeFillRect(160 - 16 * 3, j * 4 + 2, 16, 4, (j << 6) + 1);
      tft.writeFillRect(160 - 16 * 2, j * 4, 16, 4, j << 11);
      tft.writeFillRect(160 - 16, j * 4, 16, 4, (j << 11) | (j << 6) | (j)); // Gray

      // ST7735_Rect_to_queue(160 - 16 * 4, j * 4, 16, 4, j);
      // ST7735_Rect_to_queue(160 - 16 * 3, j * 4, 16, 4, j << 6);
      // ST7735_Rect_to_queue(160 - 16 * 3, j * 4 + 2, 16, 4, (j << 6) + 1);
      // ST7735_Rect_to_queue(160 - 16 * 2, j * 4, 16, 4, j << 11);
      // ST7735_Rect_to_queue(160 - 16, j * 4, 16, 4, (j << 11) | (j << 6) | j);
    }

    if (L_but->isClick() || L_but->get_inc())
    {
      if (counter == 16)
        counter = 0;
      else
        counter++;
    }

    if (R_but->isClick() || R_but->get_inc())
    {
      if (counter == 0)
        counter = 16;
      else
        counter--;
    }

    if (B_but->isClick() || B_but->get_inc())
    {
      if (counter != 16) // если не def settings
      {
        if (!chosen_GAMCTRP1)
        {
          if (GAMCTRP1[3 + counter] == 63)
            GAMCTRP1[3 + counter] = 0;
          else
            GAMCTRP1[3 + counter]++;
        }
        else
        {
          if (GMCTRN1[3 + counter] == 63)
            GMCTRN1[3 + counter] = 0;
          else
            GMCTRN1[3 + counter]++;
        }
      }
    }

    if (T_but->isClick() || T_but->get_inc())
    {
      if (counter != 16) // если не def settings
      {
        if (!chosen_GAMCTRP1)
        {
          if (GAMCTRP1[3 + counter] == 0)
            GAMCTRP1[3 + counter] = 63;
          else
            GAMCTRP1[3 + counter]--;
        }
        else
        {
          if (GMCTRN1[3 + counter] == 0)
            GMCTRN1[3 + counter] = 63;
          else
            GMCTRN1[3 + counter]--;
        }
      }
    }

    if (C_but->isClick())
    {
      if (counter != 16)
        chosen_GAMCTRP1 = !chosen_GAMCTRP1;
      else // set default gamma settings
        set_def_gamma();
    }

    if (C_but->isHolded())
      break;

    // static uint32_t timew = WDEV_NOW();
    // tft.setCursor(70, 0);
    // tft.print("fps:");
    // tft.setCursor(70, 8);
    // tft.print(((float)1000000 / (micros() - timew)), 3, 3);
    // timew = WDEV_NOW();

    tft.setCursor(0, 0);
    for (uint8_t i = 0; i < 16; i++)
    {
      tft.setTextColor(counter == i ? RED : GREEN, BLACK);

      if (!chosen_GAMCTRP1)
      {
        tft.printf("%s:0x%2x\n", reg_names_pos[i], GAMCTRP1[3 + i]);
        /*tft.print(reg_names_pos[i]);
        tft.write(':');
        tft.printHex(GAMCTRP1[3 + i], 2);
        tft.write('\n');*/
      }
      else
      {
        tft.printf("%s:0x%2x\n", reg_names_neg[i], GMCTRN1[3 + i]);
        /*tft.print(reg_names_neg[i]);
        tft.write(':');
        tft.printHex(GMCTRN1[3 + i], 2);
        tft.write('\n');*/
      }
      // tft.setTextColor(GREEN);
    }

    tft.setTextColor(counter == 16 ? RED : GREEN, BLACK);
    tft.setCursor(72 + 6, 0);
    tft.print("def");

    ST7735::displayInit(GAMCTRP1);
    ST7735::displayInit(GMCTRN1);
    // draw_settings();
    system_soft_wdt_feed();
    tft.Renderer();
    tft.Clear();
  }

  save_settings();
  tft.setRotation(0);
  tft.fillScreen(0);
  L_but->setIncfreq(10); // ������� ���������� Hz
  R_but->setIncfreq(10);
  B_but->setIncfreq(10);
  T_but->setIncfreq(10);
}

void PixelSpeedTest_app(menueItem *item, void *ptr)
{
  // item->renderTitle();
  forcedQuickUpdate();

  uint32_t time;

  while (1)
  {
    soft_updates();
    if (L_but->isClick())
      break;
    wait_queue_to_empty();

    time = WDEV_NOW();
    tft.Renderer();
    tft.Clear();
    for (uint16_t i = 0; i < 10000; i++)
      tft.writePixel(Random(0, 127), Random(26, 151), Random_16());
    wait_queue_to_empty();
    time = WDEV_NOW() - time;

    tft.setCursor(42, 17);
    tft.setTextColor(GREEN, BLACK);
    tft.printU(10000000000 / time, 6);

    tft.setCursor(0, 9);
    tft.setTextSize(1);
    tft.print("pixels per\nsecond:");
  }
}

void RectFixSpeedTest_app(menueItem *item, void *ptr)
{
  RectSpeedTest_app(item, reinterpret_cast<void *>(1));
}
void RectSpeedTest_app(menueItem *item, void *ptr)
{
  uint32_t option = reinterpret_cast<uint32_t>(ptr);
  forcedQuickUpdate();

  uint32_t time;
  while (1)
  {
    soft_updates();
    if (L_but->isClick())
      break;
    wait_queue_to_empty();

    time = WDEV_NOW();
    tft.Renderer();
    tft.Clear();
    for (uint16_t i = 0; i < 300; i++)
    {
      int16_t w, h;
      if (option)
      {
        w = 25;
        h = 25;
      }
      else
      {
        w = Random(5, 100);
        h = Random(5, 100);
      }
      tft.writeFillRectPreclipped(Random(0, 127 - w), Random(26, 151 - h), w, h, Random_16());
    }
    wait_queue_to_empty();
    time = WDEV_NOW() - time; // sum_time +=

    tft.setCursor(42, 17);
    tft.setTextColor(GREEN, BLACK);
    tft.printU(300000000 / time, 6);

    tft.setCursor(0, 9);
    tft.setTextSize(1);
    if (option)
      tft.print("25x25 rects per\nsecond:");
    else
      tft.print("random rects per\nsecond:");
  }
}

void ChipInfo_app(menueItem *item, void *ptr)
{
  tft.setRotation(3);
  forcedQuickUpdate();

  while (1)
  {
    soft_updates();
    if (L_but->isClick())
      break;

    item->renderTitle();

    tft.setCursor(0, 25);
    tft.setTextSize(1);
    tft.setTextColor(GREEN, BLACK);

    tft.printf("SDK version:\n%s\n", system_get_sdk_version()); // tft.print("SDK version:\n"); tft.print(system_get_sdk_version()); tft.write('\n');
    // tft.print("Userbin address:0x"); tft.printHex((uint32_t)system_get_userbin_addr(),4); tft.write('\n');  //
    tft.printf("Version info of boot: %d\n", system_get_boot_version());     // tft.print("Version info of boot:"); tft.printU(system_get_boot_version(),6); tft.write('\n');
    tft.printf("Time = %ud\n", system_get_time());                           // tft.print("Time = "); tft.printU(system_get_time()); tft.write('\n');
    tft.printf("RTC time = %ud\n", system_get_rtc_time());                   // tft.print("RTC time = "); tft.printU(system_get_rtc_time()); tft.write('\n');
    tft.printf("Chip id = 0x%x\n", system_get_chip_id());                    // tft.print("Chip id = 0x"); tft.printHex(system_get_chip_id()); tft.write('\n');
    tft.printf("CPU freq = %3ud MHz\n", system_get_cpu_freq());              // tft.print("CPU freq = "); tft.printU(system_get_cpu_freq(), 3); tft.print(" MHz\n");
    tft.printf("Free heap size = %5ud Byte\n", system_get_free_heap_size()); // tft.print("Free heap size = "); tft.printU(system_get_free_heap_size(),5); tft.write('\n');

    // tft.print("Flash size map = "); tft.print((int32_t)system_get_flash_size_map(),3); tft.print("Mbyte\n");

    system_soft_wdt_feed();
    tft.Renderer();
    tft.Clear();
  }

  tft.setRotation(0);
}

#include "Image_dataset.h"
void CharSpeedTest2_app(menueItem *item, void *ptr)
{
  forcedQuickUpdate();

  Image565 image;
  image.SetFlashDataset(Mario);
  // image.AllocDRAM();
  // image.LoadToRAM();

  int16_t x0 = 0, y0 = 0, dxy = 1;
  L_but->setIncfreq(50);
  C_but->setIncfreq(50);
  R_but->setIncfreq(50);
  T_but->setIncfreq(50);
  B_but->setIncfreq(50);

  while (1)
  {
    soft_updates();
    if (C_but->isHolded())
      break;
    if (C_but->isClick())
      dxy++;
    if (L_but->isClick() || L_but->get_inc())
      x0 -= dxy;
    if (R_but->isClick() || R_but->get_inc())
      x0 += dxy;
    if (T_but->isClick() || T_but->get_inc())
      y0 -= dxy;
    if (B_but->isClick() || B_but->get_inc())
      y0 += dxy;

    tft.drawImagePreclipped(x0, y0, &image);
    //  disp_Queue::Bitmap_to_queue(x0, y0, 64, 64, imagine, false);

    tft.setCursor(0, _height - 16);
    tft.setTextSize(1);
    tft.setTextColor(WHITE, BLACK);
    tft.printf("x0=%03d y0=%03d dxy=%1ud", x0, y0, dxy);
    Serial.printf("L_but->isHolded()=%d R_but->isHolded()=%d L_but->inc_value=%d R_but->inc_value=%d\n", L_but->isHolded(), R_but->isHolded(), L_but->inc_value, R_but->inc_value);

    tft.Renderer();
    tft.fillScreen(0);
    tft.Clear();
  }
  wait_queue_to_empty();

  // CharSpeedTest1_app(item, reinterpret_cast<void *>(1));
}
void CharSpeedTest1_app(menueItem *item, void *ptr)
{
  const uint32_t option = reinterpret_cast<uint32_t>(ptr);
  forcedQuickUpdate();

  uint32_t time;
  while (1)
  {
    soft_updates();
    if (L_but->isClick())
      break;
    wait_queue_to_empty();

    time = WDEV_NOW();
    tft.Renderer();
    tft.Clear();
    for (uint8_t i = 0; i != 255; i++)
      tft.drawChar(Random(0, 122 - option * 6), Random(25, 151 - option * 8), i, Random_16(), BLACK, option + 1);
    wait_queue_to_empty();
    time = WDEV_NOW() - time;

    tft.setCursor(66, 17);
    tft.setTextColor(GREEN, BLACK);
    tft.printU(255000000 / time, 6);

    tft.setCursor(0, 9);
    tft.setTextSize(1);
    if (option)
      tft.print("ASCII symbols 10x14\nper second:");
    else
      tft.print("ASCII symbols 5x7\nper second:");
  }
}

// void WriteLineSpeedTest_app(menueItem *item, void *ptr){

//}