#ifndef __MT_20120319_lang_MS__
#define __MT_20120319_lang_MS__

#include <avr/eeprom.h>

/* define index of special chars */
#define C_CH "\x01"

#define C_Zhc "\x08"
#define C_Shc "\x02"
#define C_Chc "\x03"
#define C_Rhc "\x04"
#define C_Dhc "\x05"
#define C_Thc "\x06"
#define C_Nhc "\x07"


/* define strings */
uint8_t s_welcome[] EEMEM = "vitejte";
uint8_t s_welcome_morse[] EEMEM = "VITEJTE";
uint8_t s_lesson[] EEMEM = "lekce ";
uint8_t s_correct[] EEMEM = "spravne je";
uint8_t s_congrats[] EEMEM = "gratuluji";
uint8_t s_outof[] EEMEM = "z";

uint8_t s_back[] EEMEM = "zpet";

uint8_t s_menu[] EEMEM = "menu";
uint8_t s_volume[] EEMEM = "hlasitost";
uint8_t s_pitch[] EEMEM = "ton";
uint8_t s_speed[] EEMEM = "ry" C_CH "lost";

uint8_t s_koch[] EEMEM = "ko" C_CH;
uint8_t s_farnsworth[] EEMEM = "farnswors"; //pronounciation

uint8_t s_single[] EEMEM = "znaky";
uint8_t s_groups[] EEMEM = "text";
uint8_t s_keying[] EEMEM = "kli" C_Chc "ovani";

uint8_t s_next[] EEMEM = "dal" C_Shc "i lekce";
uint8_t s_previous[] EEMEM = "p" C_Rhc "ed" C_CH "ozi lekce";

#endif /* __MT_20120319_lang_MS__ */
