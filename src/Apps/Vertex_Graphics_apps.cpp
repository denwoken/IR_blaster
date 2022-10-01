#include "Arduino.h"
extern "C"
{
#include "eagle_soc.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "my_math.h"

#include "ST7735.h"
#include "Graphics.h"
extern Graphics tft;
extern uint8_t _width;
extern uint8_t _height;

#include "Buttons.h"
extern Button *L_but;
extern Button *R_but;
extern Button *B_but;
extern Button *C_but;
extern Button *T_but;

#include "Menue/menueItem.h"

#include "Timer.h"

#include "Apps/Vertex_Graphics_apps.h"
#include "Vectors.h"
#include "SimpleCube.h"

void Render_cube_skeleton_app(menueItem *item, void *ptr)
{
  forcedQuickUpdate();

  float angle[3] = {0};
  static float ang_vel[3] = {0.9f, 0.6f, 0.3f};

  uint8_t parametr_id = 0;

  SimpleCube cube(50);
  uint32_t time = WDEV_NOW();
  while (1)
  {
    item->renderTitle();
    soft_updates();
    if (L_but->isClick())
      break;

    float t = (float)(WDEV_NOW() - time) / 1000000;
    time = WDEV_NOW();
    angle[0] += ang_vel[0] * t;
    angle[1] += ang_vel[1] * t;
    angle[2] += ang_vel[2] * t;

    cube.SetDefault();

    cube.rotateX(angle[0]);
    cube.rotateY(angle[1]);
    cube.rotateZ(angle[2]);

    cube.draw_skeleton();

    if (C_but->isClick())
      (parametr_id < 2) ? ({ parametr_id++; }) : ({ parametr_id = 0; });

    tft.setCursor(0, 136);
    tft.setTextSize(1);
    tft.setTextColor(GREEN, BLACK);

    tft.print("angular\nvelocity ");
    switch (parametr_id)
    {
    case 0:
      tft.print("X");
      break;
    case 1:
      tft.print("Y");
      break;
    case 2:
      tft.print("Z");
      break;
    }
    tft.printf(":%2.3f", (double)ang_vel[parametr_id]);
    if (T_but->isClick() || T_but->get_inc())
      ang_vel[parametr_id] += 0.1;
    if (B_but->isClick() || B_but->get_inc())
      ang_vel[parametr_id] -= 0.1;
    // for (uint16_t i = 0; i < 10000; i++)
    //   tft.writePixel(Random(0, 127), Random(26, 151), Random_16());

    tft.Renderer();
    tft.fillScreen(0); /// tft.Clear();
  }
}

void Render_cube_light_app(menueItem *item, void *ptr){};
void Render_cube_texture_app(menueItem *item, void *ptr){};
void Render_cube_free_camera_app(menueItem *item, void *ptr){};