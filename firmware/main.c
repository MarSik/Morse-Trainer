#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "dac.h"
#include "flash.h"
#include "audio.h"

void setup(void)
{
    /* setup SPI ports */
    DDRB |= _BV(MOSI) | _BV(SCK);
    PORTB = 0;

    dac_init();
    flash_init();
}

int main(void)
{
    setup();

    while(1) {
        loop();
    }

    return 0;
}
