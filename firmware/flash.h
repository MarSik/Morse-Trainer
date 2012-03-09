#ifndef __MT_flash_H_MS_20120309_
#define __MT_flash_H_MS_20120309_

/* initialization to proper state at boot */
void flash_init();

/* flash has to be selected by flash_begin before any operation takes place */
void flash_begin();
void flash_end();

/* get address <a2,a1,a0> and length <ret value> of record with id <id> */   
uint16_t flash_info(uint8_t id, uint8_t &a2, uint8_t &a1, uint8_t &a0);

/* start read sequence by seeking to proper address */
void flash_read_init(uint8_t addr2, uint8_t addr1, uint8_t addr0);

/* read one byte from flash initialized to reading state */
uint8_t flash_read();

#endif /* __MT_flash_H_MS_20120309_ */
