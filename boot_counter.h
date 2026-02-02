#ifndef _BOOT_COUNTER_H
#define _BOOT_COUNTER_H

#include "pico/stdlib.h"

#define FLASH_SECTOR_SIZE 4096
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)


uint16_t boot_counter_init(void);
uint16_t boot_counter_get(void);
void force_set_boot_counter(uint16_t count);

static uint16_t read_boot_count(void);
static void write_boot_count(uint16_t value);

#endif
