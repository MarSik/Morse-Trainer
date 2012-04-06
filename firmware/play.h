#ifndef __MT_ms_20120323_play__
#define __MT_ms_20120323_play__

#include <stdint.h>

typedef uint8_t (*getchar_f)(const uint8_t *);

uint8_t getchar_str(const uint8_t *s);
uint8_t getchar_eep(const uint8_t *s);
uint8_t getchar_pgm(const uint8_t *s);

#define FULL 0
#define COMPOSED 1

void play_characters(const uint8_t *chs, getchar_f get, uint8_t compose);
void play_character(uint8_t id);
uint8_t play_morse(const uint8_t *chs, getchar_f get);

#endif
