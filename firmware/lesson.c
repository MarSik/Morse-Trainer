#include <stdlib.h>
#include "lesson.h"
#include "morse.h"
#include "play.h"

#define RAND(xN) (rand() / (RAND_MAX / (xN)))

#define LESSON_TABLE(name) lesson_entry name[]
#define LESSON_ENTRY_LEN 2

struct _lesson_e1 {
    uint8_t startchar:6;
    uint8_t endchar:6;
    uint8_t groupmax:4;
};

struct _lesson_e2 {
    uint8_t groupmin:4;
    uint8_t effective_speed:4;
    uint8_t speed:4;
    uint8_t _reserved:4;
};

typedef union _lesson_e {
    struct _lesson_e1 e1;
    struct _lesson_e2 e2;
    uint16_t storage;
} lesson_entry;

#define LESSON(xstartchar, xendchar, xgroupmin, xgroupmax, xspeed, xeffective_speed) {.e1 = {.startchar = xstartchar, .endchar = xendchar, .groupmax = xgroupmax - 10}},\
        {.e2 = {.groupmin = xgroupmin, .effective_speed = xeffective_speed - 10, .speed = xspeed - xeffective_speed}}

#define LESSON_COUNT(name) sizeof(name)

#define T(table, id, sub) (eeprom_read_word((const uint16_t *)table + LESSON_ENTRY_LEN*id + sub))

#define LESSON_STARTCHAR(v) ((v).e1.startchar)
#define LESSON_ENDCHAR(v) ((v).e1.endchar)

#define LESSON_EFFECTIVE(v) (10 + (v).e2.effective_speed)
#define LESSON_SPEED(v) ((v).e2.speed + LESSON_EFFECTIVE(v))

#define LESSON_GROUPMIN(v) ((v).e2.groupmin)
#define LESSON_GROUPMAX(v) ((v).e1.groupmax)

LESSON_TABLE(lessons) EEMEM = {
    LESSON(0, 2, 5, 5, 18, 13),
    LESSON(3, 5, 5, 5, 18, 13),
    LESSON(0, 5, 5, 5, 18, 13),
    LESSON(6, 8, 5, 5, 19, 14),
    LESSON(0, 8, 5, 5, 19, 14),
    LESSON(9, 11, 5, 5, 19, 14),
    LESSON(0, 11, 5, 5, 20, 15),
    LESSON(12, 14, 5, 5, 20, 15),
    LESSON(0, 14, 5, 5, 20, 15),
    LESSON(15, 17, 5, 5, 20, 16),
    LESSON(0, 17, 5, 5, 20, 16),
    LESSON(18, 20, 5, 5, 20, 16),
    LESSON(0, 20, 5, 5, 20, 17),
    LESSON(21, 23, 5, 5, 20, 17),
    LESSON(0, 23, 5, 5, 20, 17),
    LESSON(24, 27, 5, 5, 20, 18),
    LESSON(0, 27, 5, 5, 20, 18), // alpha, +, =
    LESSON(28, 32, 5, 5, 20, 18), // 1 - 5
    LESSON(33, 37, 5, 5, 20, 19), // 6 - 0
    LESSON(20, 37, 5, 5, 20, 19),
    LESSON(38, 42, 5, 5, 20, 20),
    LESSON(15, 42, 5, 5, 20, 20),
    LESSON(0, 42, 5, 5, 20, 20)
};

uint8_t lesson_get(lesson_entry *table, uint8_t id,
                uint8_t *startchar, uint8_t *endchar,
                uint8_t *groupmin, uint8_t *groupmax,
                uint8_t *speed, uint8_t *effective_speed)
{
    if (id>=LESSON_COUNT(table)) return 0;

    lesson_entry e1 = {.storage = T(table, id, 0)};
    lesson_entry e2 = {.storage = T(table, id, 1)};

    *startchar = LESSON_STARTCHAR(e1);
    *endchar = LESSON_ENDCHAR(e1);
    *groupmin = LESSON_GROUPMIN(e2);
    *groupmax = LESSON_GROUPMAX(e1);
    *speed = LESSON_SPEED(e2);
    *effective_speed = LESSON_EFFECTIVE(e2);

    return 1;
}

uint8_t lesson_new(uint8_t id, uint8_t length, uint8_t *speed, uint8_t *effective_speed, uint8_t *buffer)
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
