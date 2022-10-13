
#include "Arduino.h"

extern "C"
{
#include "osapi.h"
#include "user_interface.h"
};

#include "display_drivers/driver_ST7735.h"
#include "Graphics.h"

#include "Inputs.h"
#include "Buttons.h"

#include "my_math.h"
#include "Timer.h"

#include "Menue/menueItem.h"
#include "Menue/OptionsItem.h"
#include "Menue/Init_menue.h"

#include "Pictures.h"
#include "Image565.h"
#include "LittleFS.h"
#include "FlashOptions.h"

extern struct menuelist menue;

//#include "LittleFS.h"
#include <umm_malloc/umm_heap_select.h>
#include <Esp.h>
void setup()
{
  pinMode(16, OUTPUT);
  digitalWrite(16, 0);

  Serial.begin(1000000, SERIAL_8N1, SERIAL_TX_ONLY);

  LittleFS.begin();

  init_global_options();
  tft.init();
  apply_system_settings();
  tft.fillScreen(0);

  init_inputs();
  init_HwTimer();
  init_menue();

  // delay(5000);
  //   Image565 image = Image565(image_Win1, 128, 156);
  //   image.AllocRAM();
  //   image.LoadfromCASH();
  //   for(uint8_t i = 0;i<32;i++){
  //     tft.drawImage565(0,2,&image);
  //     image.BrightnessDec();
  //     delay(40);
  //   }
  //
  //   image.FreeRAM();
}
uint32_t time1 = 0;
uint64_t summ = 0;
uint64_t count = 0;
void loop()
{
  while (1)
  {
    soft_updates();
    menue.current = menue.current->update();
    menue.current->renderSubmenues();
    tft.Renderer();
    tft.Clear();
  }

  tft.setCursor(0, 16);
  tft.setTextSize(2);
  tft.setTextColor(GREEN, BLACK);

  uint8_t ch = 0;
  while (1)
  {
    time1 = micros();

    for (uint8_t i = 0; i < 100; i++)
    {
      if (T_but->isClick())
      {
        ch++;
        tft.drawChar(0, 0, ch, GREEN, BLACK, 2);
      }
      if (B_but->isClick())
      {
        ch--;
        tft.drawChar(0, 0, ch, GREEN, BLACK, 2);
      }
      if (C_but->get_inc())
      {
        tft.setTextSize(2);
        tft.write(ch);
      }
      if (C_but->isClick())
      {
        tft.setTextSize(2);
        tft.write(ch);
      }
      if (T_but->get_inc())
      {
        ch++;
        tft.drawChar(0, 0, ch, GREEN, BLACK, 2);
      }
      if (B_but->get_inc())
      {
        ch--;
        tft.drawChar(0, 0, ch, GREEN, BLACK, 2);
      }
      tft.drawChar(Random(0, 122), Random(80, 152), i, Random_16(), BLACK, 2);
      // tft.drawChar(3, 3, i, 0xf800, BLACK, 1);
      // ST7735_Rect_to_queue(i,35,6,8,GREEN);
      // tft.fillScreen(0);
      // Input_soft_update();
      // Serial.println("update");
    }
    system_soft_wdt_feed();
    time1 = micros() - time1;
    summ += time1;
    count++;
    Serial.printf("%f\n", ((float)summ) / 100 / count);
    // delay(1);
  }
}