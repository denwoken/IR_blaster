#ifndef Inputs_H_
#define Inputs_H_

#include "Arduino.h"

#define	REG_READ(_r)	(*(volatile	uint32*)(_r))
#define	WDEV_NOW()	REG_READ(0x3ff20c00)
//#define DRAW_BUTT_STAT 

void init_inputs();


#endif // end 