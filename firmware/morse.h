#ifndef __MT_morse_20120309__
#define __MT_morse_20120309__

#include <avr/eeprom.h>

#define dit 0
#define di dit
#define DAH 1

/*
  access macros to get morse bitmask, length and represented ascii char */
#define MORSE_LEN(id) (eeprom_read_byte(morse_table + (id)*3) + 1)
#define MORSE_ID(id) eeprom_read_byte(morse_table + (id)*3 + 1)
#define MORSE_MASK(id) eeprom_read_byte(morse_table + (id)*3 + 2)

/*
  Here is a macro *hack* which enables us to write more natural definitions
  in the form of MORSE('a', DIT, DAH) without having to think about the number
  of di-dahs for different symbols.
*/

// internal macros which define 3 byte sequence to represent morse symbol [LEN-1, ID, bitmask of di-dahs (LSB is played first)]
#define MORSE_TABLE(name) uint8_t name[]
#define MORSE1(ID, D0) 0, (ID), (D0)
#define MORSE2(ID, D0, D1) 1, (ID), (D0) | ((D1) << 1)
#define MORSE3(ID, D0, D1, D2) 2, (ID), (D0) | ((D1) << 1) | ((D2) << 2)
#define MORSE4(ID, D0, D1, D2, D3) 3, (ID), (D0) | ((D1) << 1) | ((D2) << 2) | ((D3) << 3)
#define MORSE5(ID, D0, D1, D2, D3, D4) 4, (ID), (D0) | ((D1) << 1) | ((D2) << 2) | ((D3) << 3) | ((D4) << 4)
#define MORSE6(ID, D0, D1, D2, D3, D4, D5) 5, (ID), (D0) | ((D1) << 1) | ((D2) << 2) | ((D3) << 3) | ((D4) << 4) | ((D5) << 5)
#define MORSE7(ID, D0, D1, D2, D3, D4, D5, D6) 6, (ID), (D0) | ((D1) << 1) | ((D2) << 2) | ((D3) << 3) | ((D4) << 4) | ((D5) << 5) | ((D6) << 6)
#define MORSE8(ID, D0, D1, D2, D3, D4, D5, D6, D7) 7, (ID), (D0) | ((D1) << 1) | ((D2) << 2) | ((D3) << 3) | ((D4) << 4) | ((D5) << 5) | ((D6) << 6) | ((D7) << 7)

// The interim macro that simply strips the excess and ends up with the required macro
#define MORSE_X(ID, A,B,C,D,E,F,G,H, FUNC, ...) FUNC  

// The macro that the programmer uses 
#define MORSE(...) MORSE_X(__VA_ARGS__,                         \
                           MORSE8(__VA_ARGS__),                 \
                           MORSE7(__VA_ARGS__),                 \
                           MORSE6(__VA_ARGS__),                 \
                           MORSE5(__VA_ARGS__),                 \
                           MORSE4(__VA_ARGS__),                 \
                           MORSE3(__VA_ARGS__),                 \
                           MORSE2(__VA_ARGS__),                 \
                           MORSE1(__VA_ARGS__),                 \
                           )

// table of morse characters sorted in order of learning
// takes 3B per character from EEPROM
MORSE_TABLE(morse_table) EEMEM = {
    MORSE('q', DAH, DAH, di, DAH),
    MORSE('r', di, DAH, dit),
    MORSE('s', di, di, dit),
    MORSE('t', DAH),
    MORSE('+', di, DAH, di, DAH, dit),
    MORSE('=', DAH, di, di, di, DAH),
    MORSE('h', di, di, di, dit),
    MORSE('a', di, DAH),
    MORSE('p', di, DAH, DAH, dit),
    MORSE('b', DAH, di, di, dit),
    MORSE('o', DAH, DAH, DAH),
    MORSE('e', dit),
    MORSE('l', di, DAH, di, dit),
    MORSE('u', di, di, DAH),
    MORSE('c', DAH, di, DAH, dit),
    MORSE('n', DAH, dit),
    MORSE('k', DAH, di, DAH),
    MORSE('f', di, di, DAH, dit),
    MORSE('y', DAH, di, DAH, DAH),
    MORSE('g', DAH, DAH, di),
    MORSE('x', DAH, di, di, DAH),
    MORSE('d', DAH, di, dit),
    MORSE('i', di, dit),
    MORSE('m', DAH, DAH),
    MORSE('j', di, DAH, DAH, DAH),
    MORSE('v', di, di, di, DAH),
    MORSE('z', DAH, DAH, di, dit),
    MORSE('w', di, DAH, DAH),
    MORSE('1', di, DAH, DAH, DAH, DAH),
    MORSE('2', di, di, DAH, DAH, DAH),
    MORSE('3', di, di, di, DAH, DAH),
    MORSE('4', di, di, di, di, DAH),
    MORSE('5', di, di, di, di, dit),
    MORSE('6', DAH, di, di, di, dit),
    MORSE('7', DAH, DAH, di, di, dit),
    MORSE('8', DAH, DAH, DAH, di, dit),
    MORSE('9', DAH, DAH, DAH, DAH, dit),
    MORSE('0', DAH, DAH, DAH, DAH, DAH),
    MORSE('/', DAH, di, di, DAH, dit),
    MORSE(',', DAH, DAH, di, di, DAH, DAH),
    MORSE('.', di, DAH, di, DAH, di, DAH),
    MORSE('?', di, di, DAH, DAH, di, dit),
    MORSE('E', di, di, di, di, di, di, di, dit) //error
};

#endif /* __MT_morse_20120309__ */
