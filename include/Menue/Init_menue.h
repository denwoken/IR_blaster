
#pragma once
#include "Arduino.h"
#include "menueItem.h"
#include "OptionsItem.h"

void init_menue();

struct menuelist
{
    menueItem *current;
    menueItem *main;
    menueItem *Options;
    OptionsItem *buffering;
    OptionsItem *SPI_speed;
    OptionsItem *CPU_speed;
    OptionsItem *SPI_Queue;
    OptionsItem *sh_heap;
    OptionsItem *sh_fps;
    OptionsItem *bat_voltage;
    OptionsItem *Gamma_set;
    menueItem *Flash_Data;
    menueItem *Flash_Info;
    menueItem *Flash_Sectors;
    menueItem *SDFAT;
    menueItem *Games;
    menueItem *Tetris;
    menueItem *Snake;
    menueItem *VertexGraph;
    menueItem *Render_cube_skeleton;
    menueItem *Render_cube_light;
    menueItem *Render_cube_texture;
    menueItem *Render_cube_free_camera;
    menueItem *IR;
    menueItem *IR_Comm_List;
    menueItem *IR_Receive_Comm;
    menueItem *Chip_Info;

    menueItem *Tests;
    menueItem *P_S_Test;
    menueItem *Rf_S_Test;
    menueItem *R_S_Test;
    menueItem *ASCII_1_S_Test;
    menueItem *ASCII_2_S_Test;
    menueItem *WIFI;
};
extern menuelist menue;