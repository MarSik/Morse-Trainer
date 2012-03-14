#include "morse.h"

/*
  Here is a macro *hack* which enables us to write more natural definitions
  in the form of MORSE('a', DIT, DAH) without having to think about the number
  of di-dahs for different symbols.
*/

// internal macros which define 3 byte sequence to represent morse symbol [LEN-1, ID, bitmask of di-dahs (LSB is played first)]
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
    MORSE('Q', DAH, DAH, di, DAH),
    MORSE('R', di, DAH, dit),
    MORSE('S', di, di, dit),
    MORSE('T', DAH),
    MORSE('+', di, DAH, di, DAH, dit),
    MORSE('=', DAH, di, di, di, DAH),
    MORSE('H', di, di, di, dit),
    MORSE('A', di, DAH),
    MORSE('P', di, DAH, DAH, dit),
    MORSE('B', DAH, di, di, dit),
    MORSE('O', DAH, DAH, DAH),
    MORSE('E', dit),
    MORSE('L', di, DAH, di, dit),
    MORSE('U', di, di, DAH),
    MORSE('C', DAH, di, DAH, dit),
    MORSE('N', DAH, dit),
    MORSE('K', DAH, di, DAH),
    MORSE('F', di, di, DAH, dit),
    MORSE('Y', DAH, di, DAH, DAH),
    MORSE('G', DAH, DAH, di),
    MORSE('X', DAH, di, di, DAH),
    MORSE('D', DAH, di, dit),
    MORSE('I', di, dit),
    MORSE('M', DAH, DAH),
    MORSE('J', di, DAH, DAH, DAH),
    MORSE('V', di, di, di, DAH),
    MORSE('Z', DAH, DAH, di, dit),
    MORSE('W', di, DAH, DAH),
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
    MORSE(0xff, di, di, di, di, di, di, di, dit), //error
    0
};


uint8_t morse_find(uint8_t id)
{
    uint8_t idx = 0;
    
    while ((MORSE_ID(idx) != 0) && (MORSE_ID(idx) != id))
        idx++;

    return idx;          
}
