#include <avr/io.h>
#include "leds.h"

void leds_init(void)
{
    LEDS_DDR |= _BV(LED_GRN) | _BV(LED_RED) | _BV(LED_ON);
    LEDS_PORT |= _BV(LED_ON);
}
