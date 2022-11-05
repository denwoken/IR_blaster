
#pragma once
#include "Arduino.h"

#include "Graphics.h"

typedef struct Title
{
	uint8_t coord_x;
	uint8_t coord_y;
	const char *name;
	uint8_t len;
	uint16_t color;
} Title;

class menueItem
{
public:
	menueItem(void);
	void Add(menueItem *a);

	menueItem *GetChosenPtr(void);
	menueItem *GetParentPtr(void);
	void SetParentPtr(menueItem *);

	menueItem *update();
	virtual menueItem *updateClickEvents();

	void renderTitle();
	void renderSubmenues();
	virtual void render();

	bool Event_exist = 0;
	void (*Ch_ClickEvent)(menueItem *, void *);
	void SetEvent(void (*func)(menueItem *, void *))
	{
		Ch_ClickEvent = func;
		Event_exist = 1;
	}

	struct
	{
		menueItem *ParentPtr = NULL;
		menueItem **submenues = NULL;
		uint8_t len = 0;
		int8_t current = 0;
		int8_t last = 2;
	} context;

	uint8_t seq_num = 0;

	Title title;
	void SetTitle(const char *str, uint16_t col);
};
