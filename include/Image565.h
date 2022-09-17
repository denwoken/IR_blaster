#pragma once
#include"Arduino.h"



class Image565 
{
public:
	Image565(uint16_t* cash_ptr ,uint8_t width, uint8_t height);
	~Image565();
	//void SetDataPtr(uint16_t*);

	bool AllocRAM();
	void LoadfromCASH();
	void FreeRAM();


	void BrightnessDec(uint8_t br);
	void BrightnessDec();
	void BrightnessInc();
	


	uint8_t width;
	uint8_t height;
	uint16_t num_pixels;

	uint16_t* ram_ptr;
	uint16_t* cash_ptr;

	
};