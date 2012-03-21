#include <stdlib.h>
#include "lesson.h"
#include "morse.h"

#define RAND(N) (rand() / (RAND_MAX / N))

uint8_t lesson_new(uint8_t chars, uint8_t length, uint8_t group_min, uint8_t group_max, uint8_t *buffer)
{
    uint8_t idx = 0;
    uint8_t group;

    while (length>0 && length>group_min) {
        if (group_min == group_max) group = group_min;
        else group = group_min + RAND(group_max - group_min);

        while (length>0 && group>0) {
            buffer[idx++] = MORSE_ID(RAND(chars));
            length--;
            group--;
        }

        if (length-->0) buffer[idx++] = ' ';
    }

    buffer[idx] = '\0';
    return idx;
}
