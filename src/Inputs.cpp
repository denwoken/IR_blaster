#include "Inputs.h"
#include "Buttons.h"

#include "Arduino.h"
//#include "SPI.h"

extern "C"
{
#include "eagle_soc.h"
#include "gpio.h"
#include "hw_timer.h"
#include <ets_sys.h>
#include "user_interface.h"
};

Button *L_but;
Button *R_but;
Button *B_but;
Button *T_but;
Button *C_but;

void init_inputs()
{

  Button *but = (Button *)(os_malloc(5 * sizeof(Button)));

  L_but = but + 0;
  R_but = but + 1;
  B_but = but + 2;
  T_but = but + 3;
  C_but = but + 4;

  L_but->init(5);
  R_but->init(3);
  B_but->init(0);
  T_but->init(15);
  C_but->init(4);

  L_but->setDebounce(20000); // ������ �� �������� us
  R_but->setDebounce(20000);
  B_but->setDebounce(20000);
  T_but->setDebounce(20000);
  C_but->setDebounce(20000);

  L_but->setHoldtimeout(300000); // ����� �� ������ ���������� us
  R_but->setHoldtimeout(300000);
  B_but->setHoldtimeout(300000);
  T_but->setHoldtimeout(300000);
  C_but->setHoldtimeout(300000);

  L_but->setIncfreq(10); // ������� ���������� Hz
  R_but->setIncfreq(10);
  B_but->setIncfreq(10);
  T_but->setIncfreq(10);
  C_but->setIncfreq(10);
  delay(10);
};

void IRAM_ATTR intr_gpio()
{
  ETS_GPIO_INTR_DISABLE();
  uint32_t isr_reg = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, isr_reg);

  C_but->isr_update(isr_reg);
  B_but->isr_update(isr_reg);
  T_but->isr_update(isr_reg);
  L_but->isr_update(isr_reg);
  R_but->isr_update(isr_reg);

  ETS_GPIO_INTR_ENABLE();
};
