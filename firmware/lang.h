#ifndef __MT_20120319_lang_MS__
#define __MT_20120319_lang_MS__

#include <avr/eeprom.h>

/* define index of special chars */
#define C_CH "\x01"
#define C_ERROR "\xff"

/* accented characters */
#define C_Zhc "\x08"
#define C_Shc "\x02"
#define C_Chc "\x03"
#define C_Rhc "\x04"
#define C_Dhc "\x05"
#define C_Thc "\x06"
#define C_Nhc "\x07"

#define SPACE ' '
#define AUDIBLE_SPACE '\x1F'

extern uint8_t s_welcome[] EEMEM;
extern uint8_t s_welcome_morse[] EEMEM;
extern uint8_t s_lesson[] EEMEM;
extern uint8_t s_correct[] EEMEM;
extern uint8_t s_congrats[] EEMEM;
extern uint8_t s_outof[] EEMEM;

extern uint8_t s_back[] EEMEM;

extern uint8_t s_menu[] EEMEM;
extern uint8_t s_volume[] EEMEM;
extern uint8_t s_pitch[] EEMEM;
extern uint8_t s_speed[] EEMEM;

extern uint8_t s_koch[] EEMEM ;
extern uint8_t s_farnsworth[] EEMEM; //pronounciation

extern uint8_t s_single[] EEMEM;
extern uint8_t s_groups[] EEMEM;
extern uint8_t s_keying[] EEMEM;
extern uint8_t s_digrams[] EEMEM;

extern uint8_t s_next[] EEMEM;
extern uint8_t s_previous[] EEMEM;

extern uint8_t s_remove[] EEMEM;
extern uint8_t s_add[] EEMEM;

extern uint8_t s_lesson_repeat[] EEMEM;
extern uint8_t s_lesson_all[] EEMEM;

#endif /* __MT_20120319_lang_MS__ */
