#ifndef __MT_spi_20120320__
#define __MT_spi_20120320__

#include <avr/io.h>

inline uint8_t spi_transfer(uint8_t data)
{
    uint8_t clock = USICR | _BV(USITC);
    uint8_t clock_shift = USICR | _BV(USITC) | _BV(USICLK);

    USIDR = data;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    USICR = clock;
    USICR = clock_shift;

    return USIDR;
}

typedef struct {
    uint8_t data:8;
    uint8_t clk:1;
} spi_pause_t;

extern volatile spi_pause_t spi_pause_data;

inline void spi_pause(void)
{
    spi_pause_data.clk = (PORTB >> 2) & 0x1;
    spi_pause_data.data = USIDR;
}

inline void spi_unpause(void)
{
    if (spi_pause_data.clk)
        PORTB |= _BV(2);
    else
        PORTB &= ~_BV(2);
    USIDR = spi_pause_data.data;
}

#endif /* __MT_spi_20120320__ */
