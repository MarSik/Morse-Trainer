#ifndef __MT_lesson_20120321_ms__
#define __MT_lesson_20120321_ms__

#include <stdint.h>
#include <avr/eeprom.h>
#include "morse.h"
#include "play.h"

#define RAND(xN) (rand() / (RAND_MAX / (xN)))
#define LESSON_TABLE(name) uint8_t name[]
#define LESSON_ENTRY_LEN 2

extern LESSON_TABLE(lessons) EEMEM;

#define LESSON_COUNT 8 /*(sizeof(lessons)/LESSON_ENTRY_LEN)*/
#define TEACH_LAST_CHARS 5

#define LESSON_GROUPS (1 << 0)
#define LESSON_ALL (1 << 1)

#define LESSON_GROUPMAX(v) (((v) >> 4) & 0b1111)
#define LESSON_GROUPMIN(v) ((v) & 0b1111)
#define LESSON_EFFECTIVE(v) (10 + ((v) >> 4))
#define LESSON_SPEED(v) (((v) & 0b1111) + LESSON_EFFECTIVE(v))

uint8_t lesson_id(void);
uint8_t lesson_chars(void);
uint8_t lesson_chars_change(signed char offset);
uint8_t lesson_change(signed char offset);

uint8_t inline lesson_get(uint8_t *table, uint8_t id,
                uint8_t *startchar, uint8_t *endchar,
                uint8_t *groupmin, uint8_t *groupmax,
                uint8_t *speed, uint8_t *effective_speed)
{
    if (id>=LESSON_COUNT) return 0;

    uint8_t *addr = table + (id*LESSON_ENTRY_LEN);

    uint8_t e2a = eeprom_read_byte(addr);
    uint8_t e2b = eeprom_read_byte(addr + 1);

    *endchar = lesson_chars();
    *startchar = (*endchar > TEACH_LAST_CHARS) ? *endchar - TEACH_LAST_CHARS : 0;

    *groupmax = LESSON_GROUPMAX(e2a);
    *groupmin = LESSON_GROUPMIN(e2a);
    *speed = LESSON_SPEED(e2b);
    *effective_speed = LESSON_EFFECTIVE(e2b);

    return 1;
}

uint8_t lesson_new(uint8_t id, uint8_t length, uint8_t *speed, uint8_t *effective_speed, uint8_t *buffer, uint8_t flags);

#endif
