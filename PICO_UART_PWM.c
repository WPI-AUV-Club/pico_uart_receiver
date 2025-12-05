#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"

#define FULL_FORWARD_MS 1.9
#define FULL_REVERSE_MS 1.1
#define FULL_STOP_MS 1.5
#define PWM_FREQ 400
#define PWM_PIN 22

#define UART_ID uart1
#define BAUD_RATE 31250
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// GPIO         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 
// GPIO        16 17 18 19 20 21 22 23 24 25 26 27 28 29
// PWM Channel 0A 0B 1A 1B 2A 2B 3A 3B 4A 4B 5A 5B 6A 6B 7A 7B

// Two channels of the same slice run at the same frequency, but can have a different duty rate. 
// 3A -> Slice 3, channel A

uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, int duty_cycle);
int get_duty_cycle(uint freq, float pulse_len_ms);

int main(){
    //Onboard LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    //PWM Config
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint chan = pwm_gpio_to_channel(PWM_PIN);
    pwm_set_enabled(slice_num, true);
    
    //Setting PWM
    int duty_cycle = get_duty_cycle(PWM_FREQ, FULL_STOP_MS);
    pwm_set_freq_duty(slice_num, chan, PWM_FREQ, duty_cycle);

    //UART Config
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);

    //Reading UART
    while (1) {
        // Read a single character (Blocking)
        char received_char = uart_getc(UART_ID);

        // Read a specified number of bytes
        uint8_t buffer[10];
        uart_read_blocking(UART_ID, buffer, 10);
    }

    return 0;
}

int get_duty_cycle(uint freq, float pulse_len_ms) {
    float total_pulse_ms = 1.0/freq*1000;
    return (pulse_len_ms/total_pulse_ms)*100;
}

uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, int duty_cycle) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq / 4096 + (clock % (freq * 4096) != 0);
    if (divider16 / 16 == 0) divider16 = 16;

    uint32_t wrap = clock * 16 / divider16 / freq - 1;

    pwm_set_clkdiv_int_frac(slice_num, divider16/16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * duty_cycle / 100);

    return wrap;
}

