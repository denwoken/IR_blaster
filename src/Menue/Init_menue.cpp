#include "Arduino.h"
#include "Apps/Gamma&Tests.h"

#include "Apps/Flash_apps.h"
#include "Menue/Init_menue.h"
#include "FlashOptions.h"
#include "Apps/Vertex_Graphics_apps.h"

#include "FlashOptions.h"
extern Global_options Gl_options;

menuelist menue;

const char *ICACHE_FLASH_ATTR en_dis = "disable\0enable ";
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
    menue.main->SetTitle("Main", GREEN);

    menue.Options->SetTitle("Options", GREEN);
    menue.buffering->SetTitle("buffering", GREEN);
    menue.SPI_speed->SetTitle("SPI speed", GREEN);
    menue.CPU_speed->SetTitle("CPU speed", GREEN);
    menue.SPI_Queue->SetTitle("spi queue", GREEN);
    menue.sh_heap->SetTitle("show heap", GREEN);
    menue.sh_fps->SetTitle("show fps", GREEN);
    menue.bat_voltage->SetTitle("batt. voltage", GREEN);
    menue.Gamma_set->SetTitle("Gamma settings", GREEN);

    menue.Flash_Data->SetTitle("Flash", GREEN);
    menue.Flash_Info->SetTitle("Flash Info", GREEN);
    menue.Flash_Sectors->SetTitle("Flash Sectors", GREEN);
    menue.SDFAT->SetTitle("SDFAT", GREEN);

    menue.Games->SetTitle("Games", GREEN);
    menue.Tetris->SetTitle("Tetris", GREEN);
    menue.Snake->SetTitle("Snake", GREEN);
    menue.VertexGraph->SetTitle("Vertex Graphics", GREEN);
    menue.Render_cube_skeleton->SetTitle("cube skeleton", GREEN);
    menue.Render_cube_light->SetTitle("cube light", GREEN);
    menue.Render_cube_texture->SetTitle("cube texture", GREEN);
    menue.Render_cube_free_camera->SetTitle("cube free camera", GREEN);

    menue.IR->SetTitle("IR", GREEN);
    menue.IR_Comm_List->SetTitle("IR Comm List", GREEN);
    menue.IR_Receive_Comm->SetTitle("IR Receive Comm", GREEN);

    menue.Chip_Info->SetTitle("Chip Info", GREEN);

    menue.Tests->SetTitle("Tests", GREEN);
    menue.P_S_Test->SetTitle("Pixels speed", GREEN);
    menue.R_S_Test->SetTitle("Rects speed", GREEN);
    menue.Rf_S_Test->SetTitle("Rects 25x25 speed", GREEN);
    menue.ASCII_1_S_Test->SetTitle("ASCII 5x7 speed", GREEN);
    menue.ASCII_2_S_Test->SetTitle("ASCII 10x14 speed", GREEN);

    menue.WIFI->SetTitle("WIFI", GREEN);
  }

  menue.bat_voltage->SetTextContext("", "V", 0);
  menue.bat_voltage->SetDataContext(0, 2, -3, NULL);

  menue.buffering->SetTextContext("none     \0single 16\0double 8 ", "", 3);
  menue.buffering->SetDataContext(0, 2, 1, &Gl_options.buffering);
  menue.SPI_speed->SetTextContext("", "MHz", 0);
  menue.SPI_speed->SetDataContext(1, 80, 1, &Gl_options.SPI_speed);
  menue.CPU_speed->SetTextContext("", "MHz", 0);
  menue.CPU_speed->SetDataContext(80, 160, 80, &Gl_options.CPU_speed);
  menue.SPI_Queue->SetTextContext(en_dis, "", 2);
  menue.SPI_Queue->SetDataContext(0, 1, 1, &Gl_options.spi_queue);
  menue.sh_heap->SetTextContext(en_dis, "", 2);
  menue.sh_heap->SetDataContext(0, 1, 1, &Gl_options.show_heap);
  menue.sh_fps->SetTextContext(en_dis, "", 2);
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