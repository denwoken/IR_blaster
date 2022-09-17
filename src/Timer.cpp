#include "Arduino.h"

#include"Inputs.h"
#include "Buttons.h"
#include "Timer.h"

extern "C" {
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

extern Button* L_but;
extern Button* R_but;
extern Button* B_but;
extern Button* T_but;
extern Button* C_but;

#include "FlashOptions.h"
extern Global_options Gl_options;


void forcedQuickUpdate() {
	update_fps();
	update_battery();
	update_heap();
}

void update_heap() {
	if(Gl_options.show_heap){
		tft.setTextSize(1);
		tft.setTextColor(WHITE, BLACK);
		tft.setCursor(32, _height -8);
		tft.print("free heap:");
		tft.printU(system_get_free_heap_size());
	}
}

static uint32_t count_Vframes = 0;
static uint32_t ltime_fps_update = 0;
void update_fps() {
	if (Gl_options.show_fps) {
		tft.setTextSize(1);
		tft.setTextColor(WHITE, BLACK);
		tft.setCursor(0, 0);
		tft.print("fps:");
		tft.printU((1000000 * count_Vframes) / (WDEV_NOW() - ltime_fps_update), 6);
		count_Vframes = 0;
		ltime_fps_update = WDEV_NOW();
	}
}

void update_battery() {
	
	uint16_t adc_data;
	ets_intr_lock();		 //close interrupt
	system_adc_read_fast(&adc_data, 1, 4);// (adc_addr, adc_num, adc_clk_div)
	ets_intr_unlock();	 	 //open	interrupt

	//tft.setTextSize(1);
	//tft.setTextColor(WHITE, BLACK);
	//tft.setCursor(_width - 78, 140);
	//tft.printf("%02.07f\n", ((float)adc_data)*4.15 /740 );

	tft.setTextSize(1);
	tft.setTextColor(WHITE, BLACK);
	tft.setCursor(_width-50, 0);

	uint8_t percent = MAP(adc_data, 520, 730, 0, 100);////////////
	percent = Constrain(percent, 0, 100);
	tft.printR(percent, 3);//adc_data
	tft.write('%');

	tft.writeRect(_width-14, 0, 10, 7, WHITE);//114
	tft.writeFillRect(_width-16, 2,  2, 3, WHITE);//112


	uint8_t i = MAP(adc_data, 520, 730, 0, 8);///////////

	tft.writeFillRect(_width-13, 1, 8, 5, BLACK);
	tft.writeFillRect(_width-13+8-i, 1, i, 5, (i<=3)?RED:((i<=6)?YELLOW:GREEN) );
	tft.writeFillRect(_width - 13, 1, 8-i, 5, BLACK);

}

void update_1sec() {
	system_soft_wdt_feed();
	update_battery();

};

void update_100msec() {
	
	

	update_fps(); update_heap();
#ifdef DRAW_BUTT_STAT
	static struct {
		int T : 1 = 1;
		int B : 1 = 1;
		int R : 1 = 1;
		int L : 1 = 1;
		int C : 1 = 1;
	}flags;
	if (flags.T && T_but->isPressed()) { flags.T = 0; tft.drawChar(0, 0, 24, GREEN, BLACK, 1); }
	if (flags.B && B_but->isPressed()) { flags.B = 0; tft.drawChar(6, 0, 25, GREEN, BLACK, 1); }
	if (flags.C && C_but->isPressed()) { flags.C = 0; tft.drawChar(12, 0, 249, GREEN, BLACK, 1); }
	if (flags.L && L_but->isPressed()) { flags.L = 0; tft.drawChar(18, 0, 27, GREEN, BLACK, 1); }
	if (flags.R && R_but->isPressed()) { flags.R = 0; tft.drawChar(24, 0, 26, GREEN, BLACK, 1); }

	if (!flags.T && T_but->isReleased()) { flags.T = 1; tft.drawChar(0, 0, ' ', GREEN, BLACK, 1); }
	if (!flags.B && B_but->isReleased()) { flags.B = 1; tft.drawChar(6, 0, ' ', GREEN, BLACK, 1); }
	if (!flags.C && C_but->isReleased()) { flags.C = 1; tft.drawChar(12, 0, ' ', GREEN, BLACK, 1); }
	if (!flags.L && L_but->isReleased()) { flags.L = 1; tft.drawChar(18, 0, ' ', GREEN, BLACK, 1); }
	if (!flags.R && R_but->isReleased()) { flags.R = 1; tft.drawChar(24, 0, ' ', GREEN, BLACK, 1); }
#endif
	
}



void soft_updates() {
	count_Vframes++;

	static uint32_t ltime_1s = 0;
	if (WDEV_NOW() - ltime_1s >= 1000000) {
		ltime_1s = WDEV_NOW();
		update_1sec();
	}

	static uint32_t ltime_100ms = 0;
	if (WDEV_NOW() - ltime_100ms >= 100000) {

		ltime_100ms = WDEV_NOW();
		update_100msec();
	}

	/*static bool state = 0;
	state = !state;
	pinMode(16, OUTPUT);
	if (T_but->isHolded())
		digitalWrite(16, state);
	else
		digitalWrite(16, 0);*/

}

void init_HwTimer() {
	hw_timer_init(FRC1_SOURCE, 1);//FRC1_SOURCE NMI_SOURCE
	hw_timer_set_func(hw_test_timer_cb);
	TIMER_REG_WRITE(FRC1_LOAD_ADDRESS, 2 * 1000 * 80 / 16); // every 2 ms 0x20 FRC1_LOAD_ADDRESS
}

void IRAM_ATTR hw_test_timer_cb() {
	B_but->soft_update();
	R_but->soft_update();
	C_but->soft_update();
	L_but->soft_update();
	T_but->soft_update();
}