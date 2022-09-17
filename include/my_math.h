
#pragma once
#include "c_types.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

uint8_t Random(uint8_t a, uint8_t b);
uint8_t Random_8();
uint16_t Random_16();
uint32_t Random_32();
uint8_t abs_max(float a);
int16_t MAP(int16_t data, int16_t a0, int16_t b0, int16_t a1, int16_t b1);

//#define MAP( data,  a0,  b0,  a1,  b1) ((((data - a0) * ((b1 - a1)))<<10)/ (b0 - a0)) >> 10 + a1
 	



#define Constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

float siN(float a);//a>=-M_PI/2.


