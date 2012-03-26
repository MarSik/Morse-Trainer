#ifndef __MT_lesson_20120321_ms__
#define __MT_lesson_20120321_ms__

#include <stdint.h>
#include <avr/eeprom.h>
#include "morse.h"

#define RAND(xN) (rand() / (RAND_MAX / (xN)))
#define LESSON_TABLE(name) lesson_entry name[]
#define LESSON_ENTRY_LEN 4

struct _lesson_e1 {
    uint8_t startchar:6;
    uint8_t endchar:6;
    uint8_t groupmax:4;
};

struct _lesson_e2 {
    uint8_t _reserved:4;
    uint8_t groupmin:4;
    uint8_t effective_speed:4;
    uint8_t speed:4;
};

typedef union _lesson_e {
    struct _lesson_e1 e1;
    struct _lesson_e2 e2;
    uint16_t storage;
} lesson_entry;

extern LESSON_TABLE(lessons) EEMEM;

#define LESSON_COUNT(name) sizeof(name)

#define LESSON_STARTCHAR(v) (((v) >> 10) & 0b111111)
#define LESSON_ENDCHAR(v) (((v) >> 4) & 0b111111)
#define LESSON_GROUPMAX(v) ((v) & 0b1111)

#define LESSON_GROUPMIN(v) ((v) & 0b1111)
#define LESSON_EFFECTIVE(v) (10 + ((v) >> 4))
#define LESSON_SPEED(v) (((v) & 0b1111) + LESSON_EFFECTIVE(v))

uint8_t inline lesson_get(lesson_entry *table, uint8_t id,
                uint8_t *startchar, uint8_t *endchar,
                uint8_t *groupmin, uint8_t *groupmax,
                uint8_t *speed, uint8_t *effective_speed)
{
    if (id>=LESSON_COUNT(table)) return 0;

    uint8_t *addr = (uint8_t*)table + id*LESSON_ENTRY_LEN;

    uint16_t e1 = eeprom_read_word((uint16_t*)addr);
    uint8_t e2a = eeprom_read_byte(addr + 2);
    uint8_t e2b = eeprom_read_byte(addr + 3);

    *startchar = LESSON_STARTCHAR(e1);
    *endchar = LESSON_ENDCHAR(e1);
    *groupmax = LESSON_GROUPMAX(e1);

    *groupmin = LESSON_GROUPMIN(e2a);
    *speed = LESSON_SPEED(e2b);
    *effective_speed = LESSON_EFFECTIVE(e2b);

    return 1;
}

uint8_t inline lesson_new(uint8_t id, uint8_t length, uint8_t *speed, uint8_t *effective_speed, uint8_t *buffer)
{
    uint8_t idx = 0;

    uint8_t group_min, group_max, char_min, char_max;

    if (!lesson_get(lessons, id, &char_min, &char_max, &group_min, &group_max, speed, effective_speed)) return 0;

    uint8_t group;

    while (length>0 && length>group_min) {
        if (group_min == group_max) group = group_min;
        else group = group_min + RAND(1 + group_max - group_min);

        while (length>0 && group>0) {
            buffer[idx++] = MORSE_ID(char_min + RAND(1 + char_max - char_min));
            length--;
            group--;
        }

        if (length-->0) buffer[idx++] = ' ';
    }

    buffer[idx] = '\0';
    return idx;
}

uint8_t lesson_id(void);
uint8_t lesson_change(signed char offset);

#endif
