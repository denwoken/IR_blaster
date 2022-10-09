#include "Buttons.h"
#include "Inputs.h"

#include "Arduino.h"
#include "display_drivers/SPI.h"

extern "C"
{
#include "eagle_soc.h"
#include "gpio.h"
#include <ets_sys.h>
#include "user_interface.h"
};

Button::Button(){};

void Button::init(uint8_t a)
{
	pin = a;

	switch (pin)
	{
	case 4:
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
		break;
	case 5:
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
		break;
	case 3:
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
		break;
	case 0:
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
		break;
	case 15:
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
		// PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDO_U);
		// SET_PERI_REG_MASK(PERIPHS_IO_MUX_MTDO_U, PERIPHS_IO_MUX_PULLUP2);
		break;
	default:
		pinMode(pin, INPUT_PULLUP);
		break;
	};

	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(intr_gpio, NULL);
	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_ANYEDGE);
	ETS_GPIO_INTR_ENABLE();

	flags = {0};
};

void IRAM_ATTR Button::isr_update(uint32_t isr_reg)
{
	if (flags.deb_timer)
	{
		if ((isr_reg >> pin) & 1)
		{
			flags.deb_timer = 0;
			time.l_isr = WDEV_NOW(); //���������� ����� ����������
		}
	}
};

void Button::common_update()
{
	if (WDEV_NOW() - time.l_isr >= time.debounce_delay)
	{
		flags.deb_timer = 1;

		uint32_t reg = GPIO_REG_READ(GPIO_IN_ADDRESS);
		flags.cur_value = (pin == 15) ? ((reg >> pin) & 1) : !((reg >> pin) & 1);

		if (flags.cur_value)
		{							   //���� ������ ������
			flags.count_clicks++;	   //++Click
			time.pressed = WDEV_NOW(); // ���������� ����� �������
		}
		else
		{
			flags.hold_work = 0;
		}
		flags.hold_check = flags.cur_value;

		// Serial.printf("%d %d %d %d\n", reg & 1, ((reg>>15) & 1 ), ((reg >> 4) & 1), ((reg >> 5) & 1));
	}
};

void Button::soft_update()
{
	if (!flags.deb_timer)
	{
		common_update();
	}

	if (flags.hold_check)
	{
		if (WDEV_NOW() - time.pressed >= time.hold_timeout)
		{
			flags.hold_check = 0;
			flags.hold_work = 1;
		}
	}

	if (flags.hold_work)
	{
		if (WDEV_NOW() - time.hold_last_inc >= time.hold_increment)
		{
			inc_value++;
			time.hold_last_inc = WDEV_NOW();
			time.reset_inc = WDEV_NOW();
		}
	}
	else if (inc_value && (WDEV_NOW() - time.reset_inc >= 1000000))
	{
		inc_value = 0;
		time.reset_inc = WDEV_NOW();
	}
	// if(pin==3)
	// Serial.printf("%d %d %d %d %d %d\n", pin, flags.hold_work, flags.hold_check, flags.cur_value, flags.count_clicks, flags.deb_timer);
};

bool Button::isPressed()
{
	return flags.cur_value;
};

bool Button::isReleased()
{
	return !flags.cur_value;
};

bool Button::get_inc()
{

	if (inc_value)
	{
		inc_value--;
		return 1;
	}
	else
		return 0;
};

void Button::eraseClicks()
{
	flags.count_clicks = 0;
};
void Button::eraseInc()
{
	inc_value = 0;
};

bool Button::isClick()
{
	if (flags.count_clicks)
	{
		flags.count_clicks--;
		return 1;
	}
	else
		return 0;
};

bool Button::isHolded()
{
	return flags.hold_work;
};

void Button::setDebounce(uint32_t t)
{
	time.debounce_delay = t;
};

void Button::setHoldtimeout(uint32_t t)
{
	time.hold_timeout = t;
};

void Button::setIncfreq(uint32_t t)
{
	time.hold_increment = 1000000 / t;
};
