#ifndef _UART_MANAGER_H
#define _UART_MANAGER_H

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_EVEN
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define MSG_MAX_SIZE 32

enum MSG_SEVERITY {
	NORMAL,
	ERROR,
};

void init_uart();
void send_msg(char name[MSG_MAX_SIZE], enum MSG_SEVERITY);

#endif