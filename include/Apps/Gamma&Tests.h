
#pragma once

#include "Menue/menueItem.h"
void Gamma_app(menueItem *, void *);
void PixelSpeedTest_app(menueItem *, void *);
void RectFixSpeedTest_app(menueItem *, void *);
void RectSpeedTest_app(menueItem *, void *);
void ChipInfo_app(menueItem *, void *);
void CharSpeedTest2_app(menueItem *item, void *);
void CharSpeedTest1_app(menueItem *item, void *);

#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW() REG_READ(0x3ff20c00)
