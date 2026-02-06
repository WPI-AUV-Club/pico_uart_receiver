#include "uart_manager.h"
#include "boot_counter.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pwm_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


enum STATUS_FLAGS status_flag = IDLE;
char uart_buffer[BUFFER_LEN];
char received_speeds[BUFFER_LEN];
uint buffer_index = 0;


/*! \brief Interrupt handler for UART RX
 * \ingroup uart_manager
 *
 * Handle incoming UART data and write it to uart_buffer.
 * When uart_buffer is filled and a terminating char is received, copy
 * uart_buffer into the received_speeds buffer.
 * 
 * Also uses the status_flag to allow the main loop to handle errors generated
 * here as well as know when a new packet was received
 * 
 * Its bad practice to have an interrupt function this long but... call that room for improvement :3
 */
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
            memset(uart_buffer, FULL_STOP_THROTTLE, sizeof uart_buffer); //Set Buffer to full stop
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

/*! \brief Get the received_speeds buffer containing the data (motor speeds) from the last valid packet
 * \ingroup uart_manager
 *
 * \returns char pointer to received_speeds buffer
 */
char* get_received_buffer() {
    return received_speeds;
}

/*! \brief Get the current status flag and then set it to back to IDLE
 * \ingroup uart_manager
 *
 * Allows main function to know the status of the UART interrupt handler
 * This function should NOT be called without immediately handling the returned status
 * 
 * \returns Current UART RX status - MUST BE HANDLED
 */
enum STATUS_FLAGS handle_status_flag() {
    enum STATUS_FLAGS current_flag;
    current_flag = status_flag;
    status_flag = IDLE;

    return current_flag;
}

/*! \brief Initialize the UART RX and TX lines and interrupts
 * \ingroup uart_manager
 *
 * Must be called once upon initialization
 */
void init_uart() {
    //UART Config
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    //TODO: Test if enabling FIFO gives better packet loss rates
    uart_set_fifo_enabled(UART_ID, false);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
}

/*! \brief BLOCKING FUNCTION (DO NOT USE INSIDE INTERRUPT) send a message to the UART master device
 * \ingroup uart_manager
 *
 * This will block until the TX line is free, then send its message
 * All messages are formatted with a header that contains the boot id and message severity
 * 
 * Example msg formatting
 * Message: "AB>ACK:A"
 * Error: "AB>ERR=OH_NO"
 * 
 * \param msg message to send - MUST BE NULL TERMINATED, no max length but messages should be kept short to minimize TX duty cycle
 * \param severity severity of message, error, warming, normal print, ect. 
 */
void send_msg(char msg[], enum MSG_SEVERITY severity) {
    //Format boot header
    uint16_t boot_counter = boot_counter_get();
    char boot_header[4];
    boot_header[0] = (boot_counter >> 8) & 0xFF;  // high byte
    boot_header[1] = boot_counter & 0xFF;         // low byte
    boot_header[2] = '>';
    boot_header[3] = '\0';

    //Format severity header
    char severity_header[5] = "";
    if (severity == ERROR) {
        strcpy(severity_header, "ERR=");
    }

    //Allocate memory for full message length
    size_t full_msg_len = strlen(boot_header)+strlen(severity_header)+strlen(msg);
    char *full_msg = malloc(full_msg_len + 1); //+1 for null terminator
    if (full_msg == NULL) return; //Check for valid malloc

    //Concat full message
    strcpy(full_msg, boot_header);
    strcat(full_msg, severity_header);
    strcat(full_msg, msg);

    if (full_msg) { //On valid malloc, send the message, 
        uart_puts(UART_ID, full_msg);
        uart_putc(UART_ID, '\0'); //null terminator signifies packet end
        free(full_msg); // Free allocated memory
    }
}