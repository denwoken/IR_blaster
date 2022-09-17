#pragma once
#include "Arduino.h"
void update_1sec();
void update_100msec();
void soft_updates();
void update_battery();
void update_fps();
void update_heap();
void forcedQuickUpdate();

void init_HwTimer();
void IRAM_ATTR hw_test_timer_cb();






