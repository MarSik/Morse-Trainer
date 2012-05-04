#include <avr/eeprom.h>
#include "lang.h"

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
uint8_t s_digrams[] EEMEM = "dvojice";

uint8_t s_next[] EEMEM = "dal" C_Shc "i lekce";
uint8_t s_previous[] EEMEM = "p" C_Rhc "ed" C_CH "ozi lekce";

uint8_t s_remove[] EEMEM = "me" C_Nhc "e pismen";
uint8_t s_add[] EEMEM = "vice pismen";

uint8_t s_lesson_repeat[] EEMEM = "nova pismena";
uint8_t s_lesson_all[] EEMEM = "v" C_Shc "e" C_CH "na pismena";
