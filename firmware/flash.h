#ifndef __MT_flash_H_MS_20120309_
#define __MT_flash_H_MS_20120309_

#include "spi.h"

/* ports */
#define FLASH_DDR DDRA
#define FLASH_PORT PORTA
#define FLASH_CS PA0

#define FLASH_HOLD_DDR DDRA
#define FLASH_HOLD_PORT PORTA
#define FLASH_HOLD PA1

/* initialization to proper state at boot */
void flash_init(void);

/* flash has to be selected by flash_begin before any operation takes place */
void inline flash_begin(void)
{
    /* CS active low */
    FLASH_PORT &= ~_BV(FLASH_CS);
}

void inline flash_end(void)
{
    FLASH_PORT |= _BV(FLASH_CS);
}


/* save SPI state and data to allow some other SPI operation to take place */
void inline flash_pause(void)
{
    FLASH_HOLD_PORT &= ~_BV(FLASH_HOLD);

    /* save the state of currently running SPI */
    spi_pause();
}

void inline flash_unpause(void)
{
    /* restore SPI state */
    spi_unpause();

    FLASH_HOLD_PORT |= _BV(FLASH_HOLD);
}

/* get address <a2,a1,a0> and length <ret value> of record with id <id> */   
uint16_t flash_info(uint8_t id, uint8_t *a2, uint8_t *a1, uint8_t *a0);

/* start read sequence by seeking to proper address */
void flash_read_init(uint8_t addr2, uint8_t addr1, uint8_t addr0);

/* read one byte from flash initialized to reading state */
uint8_t flash_read(void);

#endif /* __MT_flash_H_MS_20120309_ */
