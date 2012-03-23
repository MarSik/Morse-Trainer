#ifndef __MT_ms_20120323_play__
#define __MT_ms_20120323_play__

#include <stdint.h>

typedef uint8_t (*getchar_f)(uint8_t *);

uint8_t getchar_str(uint8_t *s);
uint8_t getchar_eep(uint8_t *s);
uint8_t getchar_pgm(uint8_t *s);

void play_characters(uint8_t *chs, getchar_f get);
void play_character(uint8_t id);
uint8_t play_morse(uint8_t *chs, getchar_f get);


#endif
