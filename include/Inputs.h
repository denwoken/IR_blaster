#pragma once

#include "Arduino.h"
#include "Buttons.h"
#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW() REG_READ(0x3ff20c00)
//#define DRAW_BUTT_STAT

void init_inputs();

extern Button *L_but;
extern Button *R_but;
extern Button *B_but;
extern Button *C_but;
extern Button *T_but;
