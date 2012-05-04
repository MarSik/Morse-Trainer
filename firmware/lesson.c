#include <stdlib.h>
#include <avr/eeprom.h>
#include "lesson.h"
#include "morse.h"
#include "play.h"

uint8_t teaching_lesson EEMEM = 0;
uint8_t teaching_chars EEMEM = 2;

#define LESSON(xgroupmin, xgroupmax, xspeed, xeffective_speed) \
    ((xgroupmax) << 4) | xgroupmin,                                     \
    ((xeffective_speed - 10) << 4) | (xspeed - xeffective_speed)


LESSON_TABLE(lessons) EEMEM = {
    LESSON(5, 5, 18, 13),
    LESSON(5, 5, 19, 14),
    LESSON(5, 5, 20, 15),
    LESSON(5, 5, 20, 16),
    LESSON(5, 5, 20, 17),
    LESSON(5, 5, 20, 18),
    LESSON(5, 5, 20, 19),
    LESSON(5, 5, 20, 20)
};

uint8_t lesson_id(void)
{
    return eeprom_read_byte(&teaching_lesson);
}

uint8_t lesson_change(signed char offset)
{
    uint8_t lesson = lesson_id();
    if ((lesson && (offset < 0)) ||
        ((offset > 0) && (lesson < (LESSON_COUNT - 1)))) {
        lesson += offset;
        eeprom_write_byte(&teaching_lesson, lesson);
    }
    return lesson;
}

uint8_t lesson_chars(void)
{
    return eeprom_read_byte(&teaching_chars);
}

uint8_t lesson_chars_change(signed char offset)
{
    uint8_t c = lesson_chars();
    if ((c && (offset < 0)) ||
        ((offset > 0) && (c < (MORSE_TABLE_SIZE - 1)))) {
        c += offset;
        eeprom_write_byte(&teaching_chars, c);
    }
    return c;
}

uint8_t lesson_new(uint8_t id, uint8_t length, uint8_t *speed, uint8_t *effective_speed, uint8_t *buffer, uint8_t flags)
{
    uint8_t idx = 0;

    uint8_t group_min, group_max, char_min, char_max;

    if (!lesson_get(lessons, id, &char_min, &char_max, &group_min, &group_max, speed, effective_speed)) return 0;

    uint8_t group;

    if (flags & LESSON_DIGRAMS) {
        group_min = 2;
        group_max = 2;
    }

    if (flags & LESSON_ALL) char_min = 0;

    while (length>0 && length>group_min) {
        if (flags & LESSON_NO_SPACES) group = length;
        else if (group_min == group_max) group = group_min;
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
