#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/watchdog.h"
#include "pwm_manager.h"
#include "uart_manager.h"
#include "boot_counter.h"
#include "motor_mapping.h"
#include <stdio.h>

#define WATCHDOG_MAX_TIME_MS 200


unsigned char ack_index = 1;
uint64_t last_packet_received_ms;
uint64_t curr_time_ms;


/*! \brief Initialzation and main loop
 * \ingroup main
 * This is lowkey a god function and should have more code split out into smaller functions but
 * I've spent too much time on this already lol
 */
int main(){
    boot_counter_init();

    //Onboard LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    //PWM Outputs
    init_motors();
    stop_all_motors();

    //UART
    init_uart();

    //Watchdog
    if (watchdog_enable_caused_reboot()) {
        send_msg("WATCHDOG_RST", ERROR);
    }
    watchdog_enable(WATCHDOG_MAX_TIME_MS, 1);


    last_packet_received_ms = time_us_64()/1000;

    //Main loop
    while (1) {
        watchdog_update();

        //Flag a missed packet from master device and stop motors
        curr_time_ms = time_us_64()/1000;
        if (curr_time_ms - last_packet_received_ms > MAX_TIME_BETWEEN_PACKETS_MS) {
            last_packet_received_ms = curr_time_ms;
            send_msg("MISSED_PACKET", ERROR);
            stop_all_motors();
        }
        
        //Flag a incorrectly formatted packet from master device and stop motors
        enum STATUS_FLAGS status = handle_status_flag();
        if (status == INCOMPLETE_PACKET) {
            send_msg("MALFORMED_PACKET", ERROR);
            stop_all_motors();
            continue;
        } else if (status == IDLE) { //No new packet received - no action needed
            continue;
        }

        //We have received a new, valid packet
        last_packet_received_ms = curr_time_ms;
        char* motor_speeds = get_received_buffer();
        
        //Update motors with new speeds
        set_motor_speeds(motor_speeds);
        
        //Send acknowledge packet to master device with sequential counter
        char ack_msg_buffer[7];
        snprintf(ack_msg_buffer, sizeof(ack_msg_buffer), "ACK:%c", ack_index);
        send_msg(ack_msg_buffer, NORMAL);

        if (ack_index < 255) {
            ack_index++;
        } else {
            ack_index = 1;
        }
    }

    return 0;
}

