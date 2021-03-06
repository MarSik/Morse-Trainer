#ifndef __MT_interface__ms_20120326__
#define __MT_interface__ms_20120326__

#include <avr/io.h>

#define ROTARY_PIN PINB
#define ROTARY_SHIFT 4 /* how much do we have to shift to get A and B to be LSB (LSB+1) */

//volatile uint8_t interface_buttons;
#define interface_buttons GPIOR0

extern uint8_t interface_mask;
extern volatile uint8_t interface_presses;


#define KEY_A 0
#define KEY_B 1
#define ROTARY_OLD_A 2 /* old value of rotary A sensor */
#define ROTARY_OLD_B 3 /* old value of rotary B sensor */
#define BUTTON 4
#define ROTARY_NEXT 5
#define ROTARY_PREV 6
#define NONLATCHING 7

#define NONLATCHING_MODE _BV(NONLATCHING)
#define LATCHING_MODE 0

void interface_init(void);
void interface_begin(uint8_t mode, uint8_t mask);
void interface_end(void);
uint16_t timeout(uint16_t ms, uint8_t button_mask);

void interface_iambic_key(void);
void interface_standard_key(void);

#endif
