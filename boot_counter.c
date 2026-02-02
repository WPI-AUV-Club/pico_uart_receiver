#include "boot_counter.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>


static uint16_t boot_count_ram;


/*! \brief Force reset the boot counter to provided value - DEBUGGING ONLY
 * \ingroup boot_counter
 *
 * SHOULD ONLY BE USED FOR DEBUGGING PURPOSES 
 *
 * \param count 16 bit count value
 */
void force_set_boot_counter(uint16_t count) {
    write_boot_count(count);
    boot_count_ram = count;
}

/*! \brief Increment and return the boot counter on startup - call ONCE in main init 
 * \ingroup boot_counter
 *
 * Increments a sequential counter that is used on msg headers to allow master device to identify slave reboots
 * 
 * \return Unique 16 bit count value for this boot
 */
uint16_t boot_counter_init(void) {
    uint16_t count = read_boot_count();
    
    if (count == 0xFFFF) {
        count = 0x0101; //first ever boot (set to 1 as to avoid null terminators when cast to char)
    } else {
        count++;
    }
    if (count % 256 == 0) {
        count++; //Loop LSB from 0xFF to 0x01 to avoid null terminator
    }

    write_boot_count(count);
    boot_count_ram = count;
    return count;
}

/*! \brief Get the counter for this boot - boot_counter_init() must be called first
 * \ingroup boot_counter
 * 
 * \return Unique 16 bit count value for this boot
 */
uint16_t boot_counter_get(void) {
    return boot_count_ram;
}


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