#include "uart_manager.h"
#include "boot_counter.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"


void init_uart() {
    //UART Config
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);
}


void send_msg(char msg[], enum MSG_SEVERITY severity) {
    char formatted_msg[MSG_MAX_SIZE];

    uint16_t boot_counter = boot_counter_get();
    char formatted_counter[3];
    formatted_counter[0] = (boot_counter >> 8) & 0xFF;  // high byte
    formatted_counter[1] = boot_counter & 0xFF;         // low byte
    formatted_counter[2] = '\0';

    char formatted_severity[4] = severity == ERROR ? "MSG" : "ERR";

    //"AB-MSG=ACK:A"
    snprintf(formatted_msg, sizeof formatted_msg, "%s-%s:%s", formatted_counter, formatted_severity, msg);

    uart_puts(UART_ID, formatted_msg);
}