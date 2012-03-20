#include <avr/io.h>
#include "flash.h"
#include "spi.h"

#define FLASH_READ 0x0B /* followed by 3 bytes of address and one dummy byte */
#define FLASH_ENTRY_LEN 5 /* number of bytes for one character in address table */

void flash_init()
{
    FLASH_DDR |= _BV(FLASH_CS);
    FLASH_PORT |= _BV(FLASH_CS);

    FLASH_HOLD_DDR |= _BV(FLASH_HOLD);
    FLASH_HOLD_PORT |= _BV(FLASH_HOLD);
}

inline uint8_t flash_read()
{
    return spi_transfer(0);
}

inline void flash_send(uint8_t data)
{
    spi_transfer(data);
}

void flash_read_init(uint8_t addr2, uint8_t addr1, uint8_t addr0)
{
    flash_send(FLASH_READ);
    flash_send(addr2);
    flash_send(addr1);
    flash_send(addr0);
    flash_send(0x0);
}

uint16_t flash_info(uint8_t id, uint8_t *a2, uint8_t *a1, uint8_t *a0)
{
    uint16_t addr = FLASH_ENTRY_LEN*id;
    uint16_t len;

    flash_read_init(0, addr >> 8, addr & 0xff);
    *a2 = flash_read();
    *a1 = flash_read();
    *a0 = flash_read();
    len = flash_read() << 8 | flash_read();

    return len;
}


