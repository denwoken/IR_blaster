#pragma once

#include "Arduino.h"

#define GPIO_REG_READ(reg) READ_PERI_REG(PERIPHS_GPIO_BASEADDR + reg)
#define GPIO_REG_WRITE(reg, val) WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + reg, val)

#define GPIO_STATUS_W1TC_ADDRESS 0x24
#define GPIO_STATUS_ADDRESS 0x1c
#define GPIO_IN_ADDRESS 0x18

#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW() REG_READ(0x3ff20c00)

void IRAM_ATTR intr_gpio(); // IRAM_ATTR

class Button
{
public:
	Button();
	void init(uint8_t a);

	void IRAM_ATTR isr_update(uint32_t);
	void soft_update();
	void common_update();

	bool isPressed();
	bool isReleased();

	void eraseInc();
	void eraseClicks();
	bool isClick();
	bool isHolded();
	bool get_inc();

	void setDebounce(uint32_t);
	void setHoldtimeout(uint32_t);
	void setIncfreq(uint32_t t);

	struct
	{
		unsigned int deb_timer : 1;
		unsigned int cur_value : 1;
		unsigned int count_clicks : 4;

		unsigned int hold_check : 1;
		unsigned int hold_work : 1;
	} flags;
	uint8_t pin;

	struct
	{
		uint32_t current;
		uint32_t l_isr;
		uint32_t pressed;

		uint32_t debounce_delay = 40000;

		uint32_t hold_timeout = 500000;

		uint32_t hold_increment = 100000;
		uint32_t hold_last_inc;
		uint32_t reset_inc;
		// uint32_t d_hold_increment = 100000;
		// uint32_t d_hold_last_inc;
	} time;
	uint8_t inc_value; /////////
};
