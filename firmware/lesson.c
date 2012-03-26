#include <stdlib.h>
#include "lesson.h"
#include "morse.h"
#include "play.h"



#define LESSON(xstartchar, xendchar, xgroupmin, xgroupmax, xspeed, xeffective_speed) {.e1 = {.startchar = xstartchar, .endchar = xendchar, .groupmax = xgroupmax - 10}},\
        {.e2 = {.groupmin = xgroupmin, .effective_speed = xeffective_speed - 10, .speed = xspeed - xeffective_speed}}



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

