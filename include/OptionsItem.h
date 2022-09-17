

#pragma once
#include "Arduino.h"

#include "Graphics.h"
#include "menueItem.h"


class OptionsItem : public menueItem 
{
public:
	
	OptionsItem() {};
	~OptionsItem() ;
	
	menueItem* updateClickEvents();
	void render();


	void SetTextContext(const char*, const char* format, uint8_t);
	
	
	void SetDataContext(int min, int max, int inc_offset , int* GlParametr) {
		this->min = min;
		this->max = max;
		this->inc_offset = inc_offset;
		this->GlParametr = GlParametr;
		if(GlParametr) this->curr_state = *GlParametr;
	};
	
	
private:
	void Inc();
	void Dec();

	char** parametr_context;
	char* format;

	int Parametr, inc_offset = 0;
	int* GlParametr ;
	bool change_param = 0;
	int min, max, count_string_states = 0, curr_state;
};


 