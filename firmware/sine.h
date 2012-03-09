#ifndef __MT_sine_MS_20120309_
#define __MT_sine_MS_20120309_

/* one quarter (0 - 90deg) of sine function */
uint8_t sine_table[] EEMEM = {
    0,   /* 0.000000 */
    13,  /* 12.546194 */
    25,  /* 24.971561 */
    37,  /* 37.156439 */
    49,  /* 48.983479 */
    60,  /* 60.338782 */
    71,  /* 71.112990 */
    81,  /* 81.202340 */
    91,  /* 90.509668 */
    99,  /* 98.945338 */
    106, /* 106.428110 */
    113, /* 112.885922 */
    118, /* 118.256580 */
    122, /* 122.488363 */
    126, /* 125.540516 */
    127, /* 127.383645 */
};

#define sine_table_len 16

/* get sine value transposed on 0 - 255 (center at 128),
   angle values are represented by a number between 0 - 63 (step 5.625 deg) */

uint8_t inline sine(uint8_t id) {
    uint8_t q = id / sine_table_len; /* divide by number of table entries */
    id = id & (sine_table_len - 1); /* remove quadrant id from table id */

    if(q == 0) return 128 + sine_table[id];
    else if (q == 1) return 128 + sine_table[sine_table_len - id];
    else if (q == 2) return 128 - sine_table[id);
    else if (q == 3) return 128 - sine_table[sine_table_len - id];
}

#endif /* __MT_sine_MS_20120309_ */
