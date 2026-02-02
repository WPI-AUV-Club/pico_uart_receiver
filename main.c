//TODO: ADD HARDWARE LEVEL WATCHDOG

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pwm_manager.h"
#include "uart_manager.h"
#include "boot_counter.h"
#include <string.h>
#include <stdio.h>


unsigned char ack_index = 1;
uint64_t last_packet_received_ms;
uint64_t curr_time_ms;


/*! \brief Initialzation and main loop
 * \ingroup main
 * This is lowkey a god function and should have more code split out into smaller functions but
 * I've spent too much time on this already lol
 */
int main(){
    // force_set_boot_counter(0xFFFF); //TODO: REMOVE ONCE TESTED
    // force_set_boot_counter(0x01FA); //TODO: REMOVE ONCE TESTED

    boot_counter_init();

    //Onboard LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    //PWM Outputs
    init_pwm_pin(MOTOR_0);
    init_pwm_pin(MOTOR_1);
    init_pwm_pin(MOTOR_2);
    init_pwm_pin(MOTOR_3);
    init_pwm_pin(MOTOR_4);
    init_pwm_pin(MOTOR_5);
    init_pwm_pin(MOTOR_6);
    init_pwm_pin(MOTOR_7);
    set_all_pwm_pins(FULL_STOP_THROTTLE);

    //UART
    init_uart();


    last_packet_received_ms = time_us_64()/1000;

    //Main loop
    while (1) {
        //Flag a missed packet from master device and stop motors
        curr_time_ms = time_us_64()/1000;
        if (curr_time_ms - last_packet_received_ms > MAX_TIME_BETWEEN_PACKETS_MS) {
            last_packet_received_ms = curr_time_ms;
            send_msg("MISSED_PACKET", ERROR);
            set_all_pwm_pins(FULL_STOP_THROTTLE);
        }
        
        //Flag a incorrectly formatted packet from master device and stop motors
        enum STATUS_FLAGS status = handle_status_flag();
        if (status == INCOMPLETE_PACKET) {
            send_msg("MALFORMED_PACKET", ERROR);
            set_all_pwm_pins(FULL_STOP_THROTTLE);
            continue;
        } else if (status == IDLE) { //No new packet received
            continue;
        }

        //We have received a new, valid packet
        last_packet_received_ms = curr_time_ms;
        char* motor_speeds = get_received_buffer();
        
        //TODO: Move this pin mapping to a proper config file (change in main init and set_all_pwm_pins aswell)
        set_pwm_pin(MOTOR_0, motor_speeds[0]);
        set_pwm_pin(MOTOR_1, motor_speeds[1]);
        set_pwm_pin(MOTOR_2, motor_speeds[2]);
        set_pwm_pin(MOTOR_3, motor_speeds[3]);
        set_pwm_pin(MOTOR_4, motor_speeds[4]);
        set_pwm_pin(MOTOR_5, motor_speeds[5]);
        set_pwm_pin(MOTOR_6, motor_speeds[6]);
        set_pwm_pin(MOTOR_7, motor_speeds[7]);
        
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

