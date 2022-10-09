#include "Arduino.h"
#include "Apps/Gamma&Tests.h"

extern "C"
{
  //#include "eagle_soc.h"
//#include "osapi.h"
// #include <ets_sys.h>
#include "user_interface.h"
};

#include "Apps/Flash_apps.h"
#include "Menue/Init_menue.h"
#include "FlashOptions.h"
#include "Apps/Vertex_Graphics_apps.h"

#include "FlashOptions.h"
extern Global_options Gl_options;

menuelist menue;

void init_menue()
{

  Serial.printf("menues :%d \n", (sizeof(menue) / sizeof(menueItem *)));
  Serial.printf("mem_menue :%d \n", (sizeof(menue) / sizeof(menueItem *)) * sizeof(menueItem));
  Serial.printf("heap :%d \n", system_get_free_heap_size());

  {

    menue.main = new menueItem();
    menue.Options = new menueItem();
    menue.buffering = new OptionsItem();
    menue.SPI_speed = new OptionsItem();
    menue.CPU_speed = new OptionsItem();
    menue.SPI_Queue = new OptionsItem();
    menue.sh_heap = new OptionsItem();
    menue.sh_fps = new OptionsItem();
    menue.bat_voltage = new OptionsItem();
    menue.Gamma_set = new OptionsItem();
    menue.Flash_Data = new menueItem();
    menue.Flash_Info = new menueItem();
    menue.Flash_Sectors = new menueItem();
    menue.SDFAT = new menueItem();
    menue.Games = new menueItem();
    menue.Tetris = new menueItem();
    menue.Snake = new menueItem();
    menue.VertexGraph = new menueItem();
    menue.Render_cube_skeleton = new menueItem();
    menue.Render_cube_light = new menueItem();
    menue.Render_cube_texture = new menueItem();
    menue.Render_cube_free_camera = new menueItem();
    menue.IR = new menueItem();
    menue.IR_Comm_List = new menueItem();
    menue.IR_Receive_Comm = new menueItem();
    menue.Chip_Info = new menueItem();
    menue.Tests = new menueItem();
    menue.P_S_Test = new menueItem();
    menue.Rf_S_Test = new menueItem();
    menue.R_S_Test = new menueItem();
    menue.ASCII_1_S_Test = new menueItem();
    menue.ASCII_2_S_Test = new menueItem();
    menue.WIFI = new menueItem();

    menue.current = menue.main;
  }

  {
    menue.main->Add(menue.Options);
    menue.Options->Add(menue.buffering);
    menue.Options->Add(menue.SPI_speed);
    menue.Options->Add(menue.CPU_speed);
    menue.Options->Add(menue.SPI_Queue);
    menue.Options->Add(menue.sh_heap);
    menue.Options->Add(menue.sh_fps);
    menue.Options->Add(menue.bat_voltage);
    menue.Options->Add(menue.Gamma_set);

    menue.main->Add(menue.Flash_Data);
    menue.Flash_Data->Add(menue.Flash_Info);
    menue.Flash_Data->Add(menue.Flash_Sectors);
    menue.Flash_Data->Add(menue.SDFAT);

    menue.main->Add(menue.Games);
    menue.Games->Add(menue.Tetris);
    menue.Games->Add(menue.Snake);
    menue.Games->Add(menue.VertexGraph);
    menue.VertexGraph->Add(menue.Render_cube_skeleton);
    menue.VertexGraph->Add(menue.Render_cube_light);
    menue.VertexGraph->Add(menue.Render_cube_texture);
    menue.VertexGraph->Add(menue.Render_cube_free_camera);

    menue.main->Add(menue.IR);
    menue.IR->Add(menue.IR_Comm_List);
    menue.IR->Add(menue.IR_Receive_Comm);

    menue.main->Add(menue.Tests);
    menue.Tests->Add(menue.P_S_Test);
    menue.Tests->Add(menue.R_S_Test);
    menue.Tests->Add(menue.Rf_S_Test);
    menue.Tests->Add(menue.ASCII_1_S_Test);
    menue.Tests->Add(menue.ASCII_2_S_Test);

    menue.main->Add(menue.Chip_Info);

    menue.main->Add(menue.WIFI);
  }

  {
    menue.main->SetTitle(PSTR("Main"), GREEN);

    menue.Options->SetTitle(PSTR("Options"), GREEN);
    menue.buffering->SetTitle(PSTR("buffering"), GREEN);
    menue.SPI_speed->SetTitle(PSTR("SPI speed"), GREEN);
    menue.CPU_speed->SetTitle(PSTR("CPU speed"), GREEN);
    menue.SPI_Queue->SetTitle(PSTR("spi queue"), GREEN);
    menue.sh_heap->SetTitle(PSTR("show heap"), GREEN);
    menue.sh_fps->SetTitle(PSTR("show fps"), GREEN);
    menue.bat_voltage->SetTitle(PSTR("batt. voltage"), GREEN);
    menue.Gamma_set->SetTitle(PSTR("Gamma settings"), GREEN);

    menue.Flash_Data->SetTitle(PSTR("Flash"), GREEN);
    menue.Flash_Info->SetTitle(PSTR("Flash Info"), GREEN);
    menue.Flash_Sectors->SetTitle(PSTR("Flash Sectors"), GREEN);
    menue.SDFAT->SetTitle(PSTR("SDFAT"), GREEN);

    menue.Games->SetTitle(PSTR("Games"), GREEN);
    menue.Tetris->SetTitle(PSTR("Tetris"), GREEN);
    menue.Snake->SetTitle(PSTR("Snake"), GREEN);
    menue.VertexGraph->SetTitle(PSTR("Vertex Graphics"), GREEN);
    menue.Render_cube_skeleton->SetTitle(PSTR("cube skeleton"), GREEN);
    menue.Render_cube_light->SetTitle(PSTR("cube light"), GREEN);
    menue.Render_cube_texture->SetTitle(PSTR("cube texture"), GREEN);
    menue.Render_cube_free_camera->SetTitle(PSTR("cube free camera"), GREEN);

    menue.IR->SetTitle(PSTR("IR"), GREEN);
    menue.IR_Comm_List->SetTitle(PSTR("IR Comm List"), GREEN);
    menue.IR_Receive_Comm->SetTitle(PSTR("IR Receive Comm"), GREEN);

    menue.Chip_Info->SetTitle(PSTR("Chip Info"), GREEN);

    menue.Tests->SetTitle(PSTR("Tests"), GREEN);
    menue.P_S_Test->SetTitle(PSTR("Pixels speed"), GREEN);
    menue.R_S_Test->SetTitle(PSTR("Rects speed"), GREEN);
    menue.Rf_S_Test->SetTitle(PSTR("Rects 25x25 speed"), GREEN);
    menue.ASCII_1_S_Test->SetTitle(PSTR("ASCII 5x7 speed"), GREEN);
    menue.ASCII_2_S_Test->SetTitle(PSTR("ASCII 10x14 speed"), GREEN);

    menue.WIFI->SetTitle(PSTR("WIFI"), GREEN);
  }

  menue.bat_voltage->SetTextContext(PSTR(""), PSTR("V"), 0);
  menue.bat_voltage->SetDataContext(0, 2, -3, NULL);

  menue.buffering->SetTextContext(PSTR("none     \0single 16\0double 8 "), PSTR(""), 3);
  menue.buffering->SetDataContext(0, 2, 1, &Gl_options.buffering);
  menue.SPI_speed->SetTextContext(PSTR(""), PSTR("MHz"), 0);
  menue.SPI_speed->SetDataContext(1, 80, 1, &Gl_options.SPI_speed);
  menue.CPU_speed->SetTextContext(PSTR(""), PSTR("MHz"), 0);
  menue.CPU_speed->SetDataContext(80, 160, 80, &Gl_options.CPU_speed);
  menue.SPI_Queue->SetTextContext(PSTR("disable\0enable "), PSTR(""), 2);
  menue.SPI_Queue->SetDataContext(0, 1, 1, &Gl_options.spi_queue);
  menue.sh_heap->SetTextContext(PSTR("disable\0enable "), PSTR(""), 2);
  menue.sh_heap->SetDataContext(0, 1, 1, &Gl_options.show_heap);
  menue.sh_fps->SetTextContext(PSTR("disable\0enable "), PSTR(""), 2);
  menue.sh_fps->SetDataContext(0, 1, 1, &Gl_options.show_fps);

  menue.Gamma_set->SetEvent(Gamma_app);
  menue.Render_cube_skeleton->SetEvent(Render_cube_skeleton_app);
  menue.Render_cube_light->SetEvent(Render_cube_light_app);
  menue.Render_cube_texture->SetEvent(Render_cube_texture_app);
  menue.Render_cube_free_camera->SetEvent(Render_cube_free_camera_app);
  menue.P_S_Test->SetEvent(PixelSpeedTest_app);
  menue.R_S_Test->SetEvent(RectSpeedTest_app);
  menue.Rf_S_Test->SetEvent(RectFixSpeedTest_app);
  menue.ASCII_1_S_Test->SetEvent(CharSpeedTest1_app);
  menue.ASCII_2_S_Test->SetEvent(CharSpeedTest2_app);
  menue.Chip_Info->SetEvent(ChipInfo_app);
  menue.Flash_Info->SetEvent(FlashInfo_app);
}