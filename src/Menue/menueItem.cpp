
#include "Menue/menueItem.h"
#include "Arduino.h"

extern "C"
{
#include "eagle_soc.h"
//#include "gpio.h"
#include "osapi.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "Inputs.h"
#include "Buttons.h"
extern Button *L_but;
extern Button *R_but;
extern Button *B_but;
extern Button *C_but;
extern Button *T_but;
extern uint8_t _width;
extern uint8_t _height;

#include "Graphics.h"
#include "Timer.h"
extern Graphics tft;

menueItem::menueItem()
{
	SetTitle("none", WHITE);
};

void menueItem::Add(menueItem *a)
{
	context.len++;
	if (context.len == 1)
	{
		context.submenues = (menueItem **)os_malloc(context.len * sizeof(menueItem *));
		context.submenues[0] = a;
		// Serial.printf("-%s\n", (*context.submenues)->title.name);
	}
	else
	{
		menueItem **mem = (menueItem **)os_malloc(context.len * sizeof(menueItem *));
		os_memcpy(mem, context.submenues, (context.len - 1) * sizeof(menueItem *));
		mem[context.len - 1] = a;
		os_free(context.submenues);
		context.submenues = mem;
		// Serial.printf("-%s\n", context.submenues[context.len-1]->title.name);
	}
	a->SetParentPtr(this);
	a->seq_num = context.len - 1;
};

void menueItem::SetTitle(const char *str, uint16_t col)
{
	if (title.name != NULL)
		os_free(title.name);
	title.len = strlen(str);
	title.name = (char *)os_malloc(title.len + 1);
	os_memcpy(title.name, str, title.len + 1);
	title.color = col;
};

menueItem *menueItem::GetChosenPtr()
{
	if (context.len != 0)
	{
		return context.submenues[context.current];
	}
	else
	{
		return NULL;
	}
}

menueItem *menueItem::GetParentPtr()
{
	return context.ParentPtr;
};

void menueItem::SetParentPtr(menueItem *a)
{
	context.ParentPtr = a;
};

// void menueItem::SetChEvent(void (*Ev)()) {
//	Ch_ClickEvent = Ev;
// };

menueItem *menueItem::update()
{
	return GetChosenPtr()->updateClickEvents();
}

menueItem *menueItem::updateClickEvents()
{

	menueItem *parent = GetParentPtr();

	if (T_but->isClick())
	{
		if (parent->context.current > 0)
		{
			parent->context.last = parent->context.current;
			parent->context.current--;
		}
	}
	if (T_but->get_inc())
	{ // T_but->isHolded()

		if (parent->context.current > 0)
		{
			parent->context.last = parent->context.current;
			parent->context.current--;
		}
	}

	if (B_but->isClick())
	{
		if (parent->context.current < parent->context.len - 1)
		{
			parent->context.last = parent->context.current;
			parent->context.current++;
		}
	}
	if (B_but->get_inc())
	{ // B_but->isHolded()

		if (parent->context.current < parent->context.len - 1)
		{
			parent->context.last = parent->context.current;
			parent->context.current++;
		}
	}

	if (C_but->isClick() || R_but->isClick())
	{

		if (context.len != 0)
		{
			// next->SetParentPtr(this);
			tft.fillScreen(0);
			forcedQuickUpdate();
			return this; // curr_menueItem = next;
		}
		else if (Event_exist)
		{
			tft.fillScreen(0);
			Ch_ClickEvent(this, NULL);
			tft.fillScreen(0);
			forcedQuickUpdate();
			C_but->eraseClicks();
			C_but->eraseInc();
			R_but->eraseClicks();
			R_but->eraseInc();
			T_but->eraseClicks();
			T_but->eraseInc();
			B_but->eraseClicks();
			B_but->eraseInc();
			L_but->eraseClicks();
			L_but->eraseInc();
		}
	}
	if (L_but->isClick())
	{

		if (parent->GetParentPtr() != NULL)
		{
			tft.writeFillRectPreclipped(0, 8, 128, 152, BLACK); // TFT->fillScreen(0);update_battery();
			return parent->GetParentPtr();						// curr_menueItem = GetParentPtr();
		}
	}
	return parent;

	// menueItem* next = GetChosenPtr();
	// menueItem* parent = GetParentPtr();

	// if (T_but->isClick()) {
	//	if (context.current > 0){
	//		context.last = context.current;
	//		context.current--;
	//	}
	// }
	// if (T_but->get_inc()) {//T_but->isHolded()
	//
	//	if (context.current > 0){
	//		context.last = context.current;
	//		context.current--;
	//	}
	// }

	// if (B_but->isClick()) {
	//	if (context.current < context.len - 1){
	//		context.last = context.current;
	//		context.current++;
	//	}
	// }
	// if (B_but->get_inc()) {//B_but->isHolded()
	//
	//	if (context.current < context.len - 1){
	//		context.last = context.current;
	//		context.current++;
	//	}
	// }

	// if (C_but->isClick() || R_but->isClick()) {

	//	if (next->context.len != 0) {
	//		//next->SetParentPtr(this);
	//		tft.fillScreen(0); forcedQuickUpdate();
	//		return next; //curr_menueItem = next;
	//	}
	//	else if(next->Event_exist) {
	//		tft.fillScreen(0);
	//		next->Ch_ClickEvent(next,NULL);
	//		tft.fillScreen(0); forcedQuickUpdate();
	//		C_but->eraseClicks(); C_but->eraseInc();
	//		R_but->eraseClicks(); R_but->eraseInc();
	//		T_but->eraseClicks(); T_but->eraseInc();
	//		B_but->eraseClicks(); B_but->eraseInc();
	//		L_but->eraseClicks(); L_but->eraseInc();
	//	}
	//}
	// if (L_but->isClick() ) {
	//
	//	if (parent != NULL) {
	//		tft.writeFillRectPreclipped(0, 8, 128, 152,BLACK);//TFT->fillScreen(0);update_battery();
	//		return parent;//curr_menueItem = GetParentPtr();
	//	}
	//
	//}
	// return this;
};

void menueItem::renderTitle()
{
	uint8_t pix_len = 2 * 6 * title.len;
	if (_width < pix_len)
	{
		tft.setTextSize(1);
		tft.setCursor((_width - pix_len / 2) / 2, 9 + 4); // strlen
		tft.writeFillRectPreclipped((_width - pix_len / 2) / 2, 9 + 4 + 8 + 2, pix_len / 2, 2, title.color);
	}
	else
	{
		tft.setTextSize(2);
		tft.setCursor((_width - pix_len) / 2, 9); // strlen
	}
	tft.setTextColor(title.color, BLACK);
	tft.print(title.name);
}

void menueItem::render()
{
	tft.setTextSize(1);
	tft.setCursor((_width - 1 * 6 * (2 + title.len)) / 2, 10 + 8 + 18 + seq_num * 12);

	if (GetParentPtr()->context.current == seq_num)
	{
		tft.setTextColor(RED, BLACK);
		tft.write(26); //->
	}
	else
		tft.write(' ');

	tft.setTextColor(title.color, BLACK);
	tft.print(const_cast<const char *>(title.name));

	if (GetParentPtr()->context.current == seq_num)
	{
		tft.setTextColor(RED, BLACK);
		tft.write(27); //<-
	}
	else
		tft.write(' ');
}

void menueItem::renderSubmenues()
{
	renderTitle();
	for (uint8_t i = 0; i < context.len; i++)
	{

		context.submenues[i]->render();
	}
}
