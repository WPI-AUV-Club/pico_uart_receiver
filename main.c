#include "pico/stdlib.h"
#include "pwm_manager.h"
#include "uart_manager.h"
#include "boot_counter.h"
#include <string.h>

#define BUFFER_LEN 8
#define TERMINATE_CHAR 255

#define MOTOR_0 2
#define MOTOR_1 3
#define MOTOR_2 4
#define MOTOR_3 5
#define MOTOR_4 6
#define MOTOR_5 7
#define MOTOR_6 8
#define MOTOR_7 9

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

    init_uart();

    //Create a buffer that will hold all of the recieved speeds, and once all of them are recieved update PWM outputs
    char uart_buffer[BUFFER_LEN];
    uint buffer_index = 0;
    unsigned char ack_index = 1;

    while (1) {
        char received_char = uart_getc(UART_ID);

        if (received_char == TERMINATE_CHAR) {
            if (buffer_index == BUFFER_LEN) { 
                set_pwm_pin(MOTOR_0, uart_buffer[0]);
                set_pwm_pin(MOTOR_1, uart_buffer[1]);
                set_pwm_pin(MOTOR_2, uart_buffer[2]);
                set_pwm_pin(MOTOR_3, uart_buffer[3]);
                set_pwm_pin(MOTOR_4, uart_buffer[4]);
                set_pwm_pin(MOTOR_5, uart_buffer[5]);
                set_pwm_pin(MOTOR_6, uart_buffer[6]);
                set_pwm_pin(MOTOR_7, uart_buffer[7]);

                char ack_msg_buffer[6];
                snprintf(ack_msg_buffer, sizeof(ack_msg_buffer), "ACK:%d", ack_index);
                send_msg(ack_msg_buffer, NORMAL);
                ack_index++;
            } else {
                send_msg("INV BUFFER SIZE @ PCK END", ERROR);
            }

            memset(uart_buffer, 0, sizeof uart_buffer); //Clear Buffer
            buffer_index = 0;

            continue;
        } else {
            if (buffer_index == BUFFER_LEN) {
                send_msg("TERMINATING CHAR NOT RECIEVED", ERROR);
                buffer_index = BUFFER_LEN+1;
                continue;
            }
            if (buffer_index >= BUFFER_LEN) continue;

            uart_buffer[buffer_index] = received_char;
            buffer_index++;
        }
    }

    //Writing UART
    // uart_putc_raw(UART_ID, 0x90);

    return 0;
}

