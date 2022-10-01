

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
extern Graphics tft;

#include "Timer.h"

#include "FlashOptions.h"
extern Global_options Gl_options;

#include "Menue/menueItem.h"
#include "Menue/OptionsItem.h"

OptionsItem::~OptionsItem()
{
	delete[] format;
	if (parametr_context != NULL)
	{
		for (uint8_t i = 0; i < count_string_states; i++)
			os_free(parametr_context[i]);

		os_free(parametr_context);
	}
}

menueItem *OptionsItem::updateClickEvents()
{
	menueItem *parent = GetParentPtr();

	if (change_param == 0)
	{
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
			else
			{
				if (inc_offset != -3)
					change_param = 1;
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
	}
	else
	{
		if (T_but->isClick() || T_but->get_inc())
			Inc();
		if (B_but->isClick() || B_but->get_inc())
			Dec();
		C_but->eraseClicks();
		R_but->eraseClicks();

		if (L_but->isClick())
		{
			change_param = 0;
			*GlParametr = curr_state;
			save_settings();
			apply_system_settings();
			tft.fillScreen(0);
			forcedQuickUpdate();
		}

		return parent;
	}
};

void OptionsItem::render()
{
	tft.setTextSize(1);
	tft.setCursor(0, 10 + 8 + 18 + seq_num * 12);

	if (GetParentPtr()->context.current == seq_num)
	{
		tft.setTextColor(RED, BLACK);
		tft.write(26); //->
	}
	else
		tft.write(' ');

	tft.setTextColor(title.color, BLACK);
	tft.print(const_cast<const char *>(title.name));

	tft.setTextColor(WHITE);
	if (GetParentPtr()->context.current == seq_num && change_param == 1)
		tft.setTextColor(RED, BLACK);

	tft.setCursor(6 + 6 * strlen(title.name) + 6, 10 + 8 + 18 + seq_num * 12);
	if (count_string_states)
		tft.printf("\t\t%s", parametr_context[curr_state]);
	else
	{
		if (inc_offset > 0 || inc_offset == -2)
			tft.printf("\t\t%4ud%s", curr_state, format);
		if (inc_offset == -3)
		{
			uint16_t adc_data;
			ets_intr_lock();					   // lock interrupt
			system_adc_read_fast(&adc_data, 1, 4); // (adc_addr, adc_num, adc_clk_div)
			ets_intr_unlock();					   // unlock interrupt
			tft.printf("%1.2f%s", ((float)adc_data) * 4.15 / 740, format);
		}
	}
}

void OptionsItem::SetTextContext(const char *str, const char *format, uint8_t c)
{
	this->format = new char[strlen(format) + 1];
	os_memcpy(this->format, format, strlen(format) + 1);

	count_string_states = c;

	if (c)
		parametr_context = (char **)os_malloc(c * sizeof(char *));
	for (uint8_t i = 0; i < c; i++)
	{
		uint8_t len = strlen(str) + 1;
		parametr_context[i] = (char *)os_malloc(len * sizeof(char));
		os_memcpy(parametr_context[i], str, len);
		str += len;
	}
}

void OptionsItem::Inc()
{

	if (curr_state + inc_offset <= max)
	{
		curr_state += inc_offset;
	}
}

void OptionsItem::Dec()
{

	if (curr_state - inc_offset >= min)
	{
		curr_state -= inc_offset;
	}
}
