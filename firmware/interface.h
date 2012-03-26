#ifndef __MT_interface__ms_20120326__
#define __MT_interface__ms_20120326__

#include <avr/io.h>

//volatile uint8_t interface_buttons;
#define interface_buttons GPIOR0

extern uint8_t interface_mask;
extern volatile uint8_t interface_presses;


#define KEY_A 0
#define KEY_B 1
#define BUTTON 2
#define PRESS_BREAKS 7

void interface_init(void);
void interface_begin(void);
void interface_end(void);
uint16_t timeout(uint16_t ms);

#endif
