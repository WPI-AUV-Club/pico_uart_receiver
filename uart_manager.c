#include "uart_manager.h"
#include "boot_counter.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


enum STATUS_FLAGS status_flag = IDLE;

char uart_buffer[BUFFER_LEN];
char received_speeds[BUFFER_LEN];
uint buffer_index = 0;


void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {        
        uint8_t received_char = uart_getc(UART_ID);

        if (received_char == TERMINATE_CHAR) {
            if (buffer_index == BUFFER_LEN) {
                status_flag = VALID_PACKET;
                memcpy(received_speeds, uart_buffer, sizeof uart_buffer);
            } else {
                status_flag = INCOMPLETE_PACKET;
            }
            memset(uart_buffer, 127, sizeof uart_buffer); //Set Buffer to full stop
            buffer_index = 0;

            continue;
        }
            
        if (buffer_index >= BUFFER_LEN) {
            status_flag = INCOMPLETE_PACKET;
            continue;
        }

        uart_buffer[buffer_index] = received_char;
        buffer_index++;
    }
}


char* get_received_buffer() {
    return received_speeds;
}


enum STATUS_FLAGS handle_status_flag() {
    enum STATUS_FLAGS current_flag;
    current_flag = status_flag;
    status_flag = IDLE;

    return current_flag;
}


void init_uart() {
    //UART Config
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
}


void send_msg(char msg[], enum MSG_SEVERITY severity) {
    char msg_header[8];

    uint16_t boot_counter = boot_counter_get();
    char boot_counter_format[3];
    boot_counter_format[0] = (boot_counter >> 8) & 0xFF;  // high byte
    boot_counter_format[1] = boot_counter & 0xFF;         // low byte
    boot_counter_format[2] = '\0';

    //"AB-MSG=ACK:A"
    if (severity == ERROR) {
        sprintf(msg_header, "%s-%s=", boot_counter_format, "ERR");
    } else {
        sprintf(msg_header, "%s=", boot_counter_format);
    }

    char *full_msg = safe_concat(msg_header, msg);
    if (full_msg) {
        uart_puts(UART_ID, full_msg);
        free(full_msg); // Free allocated memory
    }
}

static char* safe_concat(const char *s1, const char *s2) {
    // 1. Calculate required size: length1 + length2 + null-terminator + newline
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 2);

    // 2. Check for malloc failure
    if (result == NULL) return NULL;

    // 3. Perform safe concatenation
    strcpy(result, s1);
    strcat(result, s2);
    strcat(result, "\n");

    return result; // Caller must free() this memory
}