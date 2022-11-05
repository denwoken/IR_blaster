#pragma once
#include "Arduino.h"
#include "Image_dataset.h"

//#define MAX_BRIGHTNESS 31
class Image565
{
public:
	Image565(Image_dataset &dataset);
	Image565(){};
	~Image565() { FreeRAM(); };
	void SetFlashDataset(Image_dataset &dataset);

	bool AllocIRAM();
	bool AllocDRAM();
	void LoadToRAM();
	void FreeRAM();

	// void Brightness(uint8_t);
	// void BrightnessDec(uint8_t);
	// void BrightnessInc(uint8_t);
	// void BrightnessDec();
	// void BrightnessInc();

	u16 getPixel(u16 x, u16 y);

	uint16_t width;
	uint16_t height;
	uint32_t num_pixels;
	// int8_t brightness = MAX_BRIGHTNESS;

	bool DataInRam = false;
	uint16_t *Ram_ptr = NULL;
	uint16_t *Flash_ptr = NULL;
};
