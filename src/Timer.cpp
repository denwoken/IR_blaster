#include "Arduino.h"

#include "Timer.h"

extern "C"
{
#include "eagle_soc.h"
#include "gpio.h"
#include "hw_timer.h"
#include <ets_sys.h>
#include "user_interface.h"
};

#include "my_math.h"
#include "Graphics.h"
extern Graphics tft;
extern uint8_t _width;
extern uint8_t _height;

#include "FlashOptions.h"
extern Global_options Gl_options;

#include "Buttons.h"
extern Button *L_but;
extern Button *R_but;
extern Button *B_but;
extern Button *T_but;
extern Button *C_but;

// update at screen static values (rarely updated)
void forcedQuickUpdate()
{
	draw_fps();
	draw_battery();
	draw_heap();
}

// update_*** -> updating the values
// draw_*** -> sending data to display

static uint32_t free_heap_size;
void update_heap()
{
	if (Gl_options.show_heap)
	{
		free_heap_size = system_get_free_heap_size();
	}
}
void draw_heap()
{
	if (Gl_options.show_heap)
	{
		tft.setTextSize(1);
		tft.setTextColor(WHITE, BLACK);
		tft.setCursor(32, _height - 8);
		tft.print("free heap:");
		tft.printU(free_heap_size);
	}
}

static uint32_t count_Vframes = 0;
static uint32_t ltime_fps_update = 0;
static uint16_t fps;
void update_fps()
{
	if (Gl_options.show_fps)
	{
		fps = (1000000 * count_Vframes) / (WDEV_NOW() - ltime_fps_update);
		count_Vframes = 0;
		ltime_fps_update = WDEV_NOW();
	}
}
void draw_fps()
{
	if (Gl_options.show_fps)
	{

		tft.setTextSize(1);
		tft.setTextColor(WHITE, BLACK);
		tft.setCursor(0, 0);
		tft.print("fps:");
		tft.printU(fps, 6);
	}
}

static uint8_t battery_charge;
static uint8_t battery_im_offset;
void update_battery()
{

	uint16_t adc_data;
	ets_intr_lock();					   // close interrupt
	system_adc_read_fast(&adc_data, 1, 4); // (adc_addr, adc_num, adc_clk_div)
	ets_intr_unlock();					   // open	interrupt

	uint8_t percent = MAP(adc_data, 520, 730, 0, 100); ////////////
	battery_charge = Constrain(percent, 0, 100);

	battery_im_offset = MAP(adc_data, 520, 730, 0, 8); ///////////
}

void draw_battery()
{
	tft.setTextSize(1);
	tft.setTextColor(WHITE, BLACK);
	tft.setCursor(_width - 50, 0);

	tft.printR(battery_charge, 3); // adc_data
	tft.write('%');

	tft.writeRect(_width - 14, 0, 10, 7, WHITE);	// 114
	tft.writeFillRect(_width - 16, 2, 2, 3, WHITE); // 112

	uint8_t i = battery_im_offset; //
	// tft.writeFillRect(_width - 13, 1, 8, 5, BLACK);
	tft.writeFillRect(_width - 13 + 8 - i, 1, i, 5, (i <= 3) ? RED : ((i <= 6) ? YELLOW : GREEN));
	tft.writeFillRect(_width - 13, 1, 8 - i, 5, BLACK);
}

// soft timer, every >=1000 milliseconds
void update_1sec()
{
	system_soft_wdt_feed();
};

// soft timer, every >=100 milliseconds
void update_100msec()
{
	// Serial.println(fps);
	update_battery();
	update_fps();
	update_heap();

#ifdef DRAW_BUTT_STAT
	static struct
	{
		int T : 1 = 1;
		int B : 1 = 1;
		int R : 1 = 1;
		int L : 1 = 1;
		int C : 1 = 1;
	} flags;
	if (flags.T && T_but->isPressed())
	{
		flags.T = 0;
		tft.drawChar(0, 0, 24, GREEN, BLACK, 1);
	}
	if (flags.B && B_but->isPressed())
	{
		flags.B = 0;
		tft.drawChar(6, 0, 25, GREEN, BLACK, 1);
	}
	if (flags.C && C_but->isPressed())
	{
		flags.C = 0;
		tft.drawChar(12, 0, 249, GREEN, BLACK, 1);
	}
	if (flags.L && L_but->isPressed())
	{
		flags.L = 0;
		tft.drawChar(18, 0, 27, GREEN, BLACK, 1);
	}
	if (flags.R && R_but->isPressed())
	{
		flags.R = 0;
		tft.drawChar(24, 0, 26, GREEN, BLACK, 1);
	}

	if (!flags.T && T_but->isReleased())
	{
		flags.T = 1;
		tft.drawChar(0, 0, ' ', GREEN, BLACK, 1);
	}
	if (!flags.B && B_but->isReleased())
	{
		flags.B = 1;
		tft.drawChar(6, 0, ' ', GREEN, BLACK, 1);
	}
	if (!flags.C && C_but->isReleased())
	{
		flags.C = 1;
		tft.drawChar(12, 0, ' ', GREEN, BLACK, 1);
	}
	if (!flags.L && L_but->isReleased())
	{
		flags.L = 1;
		tft.drawChar(18, 0, ' ', GREEN, BLACK, 1);
	}
	if (!flags.R && R_but->isReleased())
	{
		flags.R = 1;
		tft.drawChar(24, 0, ' ', GREEN, BLACK, 1);
	}
#endif
}

// soft timer, every loop cycle
void soft_updates()
{
	count_Vframes++;

	static uint32_t ltime_1s = 0;
	if (WDEV_NOW() - ltime_1s >= 1000000)
	{
		ltime_1s = WDEV_NOW();
		update_1sec();
	}

	static uint32_t ltime_100ms = 0;
	if (WDEV_NOW() - ltime_100ms >= 100000)
	{
		ltime_100ms = WDEV_NOW();
		update_100msec();
	}

	forcedQuickUpdate();
}

// initialize hardware timer
void init_HwTimer()
{
	hw_timer_init(FRC1_SOURCE, 1); // FRC1_SOURCE NMI_SOURCE
	hw_timer_set_func(hw_test_timer_cb);
	TIMER_REG_WRITE(FRC1_LOAD_ADDRESS, 2 * 1000 * 80 / 16); // every 2 ms 0x20 FRC1_LOAD_ADDRESS
}

// harware timer function (every 2 milliseconds) 500 Hz
void IRAM_ATTR hw_test_timer_cb()
{
	B_but->soft_update();
	R_but->soft_update();
	C_but->soft_update();
	L_but->soft_update();
	T_but->soft_update();
}