//TODO: ADD WATCHDOG

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pwm_manager.h"
#include "uart_manager.h"
#include "boot_counter.h"
#include <string.h>
#include <stdio.h>

//Motor GPIO Pin Mapping
#define MOTOR_0 2
#define MOTOR_1 3
#define MOTOR_2 4
#define MOTOR_3 5
#define MOTOR_4 6
#define MOTOR_5 7
#define MOTOR_6 8
#define MOTOR_7 9

#define MAX_TIME_BETWEEN_PACKETS 60


unsigned char ack_index = 1;
uint64_t last_packet_received_ms;
uint64_t curr_time_ms;

int main(){
    boot_counter_init();

    //Onboard LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    init_pwm_pin(MOTOR_0);
    init_pwm_pin(MOTOR_1);
    init_pwm_pin(MOTOR_2);
    init_pwm_pin(MOTOR_3);
    init_pwm_pin(MOTOR_4);
    init_pwm_pin(MOTOR_5);
    init_pwm_pin(MOTOR_6);
    init_pwm_pin(MOTOR_7);

    set_pwm_pin(MOTOR_0, 127);
    set_pwm_pin(MOTOR_1, 127);
    set_pwm_pin(MOTOR_2, 127);
    set_pwm_pin(MOTOR_3, 127);
    set_pwm_pin(MOTOR_4, 127);
    set_pwm_pin(MOTOR_5, 127);
    set_pwm_pin(MOTOR_6, 127);
    set_pwm_pin(MOTOR_7, 127);

    init_uart();

    last_packet_received_ms = time_us_64()/1000;


    while (1) {
        curr_time_ms = time_us_64()/1000;
        if (curr_time_ms - last_packet_received_ms > MAX_TIME_BETWEEN_PACKETS) {
            last_packet_received_ms = curr_time_ms;
            send_msg("MISSED_PACKET", ERROR);
        }
        
        enum STATUS_FLAGS status = handle_status_flag();
        if (status == INCOMPLETE_PACKET) {
            send_msg("MALFORMED_PACKET", ERROR);
            continue;
        } else if (status == IDLE) {
            continue;
        }

        last_packet_received_ms = curr_time_ms;
        char* motor_speeds = get_received_buffer();
        
        set_pwm_pin(MOTOR_0, motor_speeds[0]);
        set_pwm_pin(MOTOR_1, motor_speeds[1]);
        set_pwm_pin(MOTOR_2, motor_speeds[2]);
        set_pwm_pin(MOTOR_3, motor_speeds[3]);
        set_pwm_pin(MOTOR_4, motor_speeds[4]);
        set_pwm_pin(MOTOR_5, motor_speeds[5]);
        set_pwm_pin(MOTOR_6, motor_speeds[6]);
        set_pwm_pin(MOTOR_7, motor_speeds[7]);
        
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

