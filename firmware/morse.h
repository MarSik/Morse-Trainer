#ifndef __MT_morse_20120309__
#define __MT_morse_20120309__

#include <avr/eeprom.h>

#define dit 0
#define di dit
#define DAH 1

#define DIT_LEN 1
#define DAH_LEN 3
#define SPACE_LEN 1
#define LETTER_SPACE_LEN 3
#define WORD_SPACE_LEN 7

uint8_t morse_find(uint8_t id);

/*
  access macros to get morse bitmask, length and represented ascii char */
#define MORSE_LEN(idx) (eeprom_read_byte(morse_table + (idx)*3) + 1)
#define MORSE_ID(idx) eeprom_read_byte(morse_table + (idx)*3 + 1)
#define MORSE_MASK(idx) eeprom_read_byte(morse_table + (idx)*3 + 2)

#define MORSE_TABLE(name) uint8_t name[]
extern MORSE_TABLE(morse_table) EEMEM;

#endif /* __MT_morse_20120309__ */
