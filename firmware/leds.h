#ifndef __MT_20120320__ms__
#define __MT_20120320__ms__

#define LEDS_DDR DDRA
#define LEDS_PORT PORTA
#define LEDS_PIN PINA

#define LED_ON PA3
#define LED_RED PA6
#define LED_GRN PA7

void leds_init(void);

inline void led_on(uint8_t led)
{
    LEDS_PORT |= _BV(led);
}

inline void led_off(uint8_t led)
{
    LEDS_PORT &= ~_BV(led);
}

inline void led_toggle(uint8_t led)
{
    LEDS_PIN |= _BV(led);
}


#endif /* __MT_20120320__ms__ */
