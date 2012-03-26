#include "spi.h"

volatile uint8_t spi_pause_data;
volatile uint8_t spi_pause_clk;

uint8_t spi_transfer(uint8_t data)
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
