#ifndef _UART_MANAGER_H
#define _UART_MANAGER_H

#define UART_ID uart0
#define BAUD_RATE 38400
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_EVEN
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define MSG_MAX_SIZE 32
#define BUFFER_LEN 8
#define TERMINATE_CHAR 255

#define MAX_TIME_BETWEEN_PACKETS_MS 60


enum MSG_SEVERITY {
	NORMAL,
	ERROR,
};
enum STATUS_FLAGS {
	IDLE,
	VALID_PACKET,
	INCOMPLETE_PACKET,
};


void init_uart();
void send_msg(char msg[], enum MSG_SEVERITY);
char* get_received_buffer();
void on_uart_rx();
enum STATUS_FLAGS handle_status_flag();
static char* safe_concat(const char *s1, const char *s2);

#endif