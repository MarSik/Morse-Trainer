#ifndef __MT_lesson_20120321_ms__
#define __MT_lesson_20120321_ms__

#include <stdint.h>
#include <avr/eeprom.h>


uint8_t lesson_new(uint8_t id, uint8_t length, uint8_t *speed, uint8_t *effective_speed, uint8_t *buffer);

#endif
