#include "boot_counter.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

static uint16_t boot_count_ram;

/* Read boot counter from flash */
static uint16_t read_boot_count(void) {
    const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    uint16_t value;
    memcpy(&value, flash_ptr, sizeof(value));
    return value;
}

/* Write boot counter to flash (erase + program) */
static void write_boot_count(uint16_t value) {
    uint8_t buffer[FLASH_SECTOR_SIZE];
    memset(buffer, 0xFF, sizeof(buffer));

    memcpy(buffer, &value, sizeof(value));

    uint32_t ints = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_SECTOR_SIZE);

    restore_interrupts(ints);
}

/* Call once at boot */
uint16_t boot_counter_init(void) {
    uint16_t count = read_boot_count();

    if (count == 0xFFFF) {
        count = 0;   // first ever boot
    } else {
        count++;
    }

    write_boot_count(count);
    boot_count_ram = count;
    return count;
}

/* Optional getter */
uint16_t boot_counter_get(void) {
    return boot_count_ram;
}
