#include "my_math.h"
#include "osapi.h"
#include "c_types.h"

float siN(float x) {
	x -= 2*M_PI*(	(int)(x/(2*M_PI))	);
	int flag=1;
	while(x>M_PI/2){x-=M_PI;flag=-flag;}
	if(flag==-1)x=-x;

  float x3=x*x*x/6;
  float x5=x3*x*x/20;
  float res = x - x3 + x5;
  return res;
}

uint8_t Random(uint8_t a, uint8_t b) {
	uint8_t temp;
	os_get_random(&temp, 1);
	return  (uint8_t)(	(  ((((uint32_t)temp)<<16)*(b+1-a))>>24	) + a	);
	//(uint8_t)(	(((float)temp) / 255) * (b - a) + a	);
};

uint8_t Random_8(){
	uint8_t temp;
	os_get_random(&temp, 1);
	return  temp;
};

uint16_t Random_16() {
	uint8_t temp[2];
	os_get_random(temp, 2);
	return  *((uint16_t*)temp);
};

uint32_t Random_32() {
	uint8_t temp[4];
	os_get_random(temp, 4);
	return  *((uint32_t*)temp);
};

uint8_t abs_max(float a) {
	uint8_t res = (int)a;
	if (a - (float)(res) > 0.5) {
		res++;
	}
	return res;
};


int16_t MAP(int16_t data, int16_t a0, int16_t b0, int16_t a1, int16_t b1) {
	int32_t c = (data - a0) * ((b1 - a1));
	c = (c << 10) / (b0 - a0);
	return (c >> 10) + a1;
}


