#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#include <stdbool.h>
#include "tm4c123gh6pm.h"		// vendor provided header
#include "ring_buffer.h"

void UART0_Handler(void);
void uart0_interrupt_initialization(void);
void uart0_interrupt_enable_transmit(void);
void uart0_interrupt_disable_transmit(void);
void uart0_interrupt_clear_receive(void);
void uart0_interrupt_clear_transmit(void);
bool uart0_interrupt_get_char(struct ring_buffer* rb, unsigned char* c);
void uart0_interrupt_send_char(struct ring_buffer* rb, unsigned char c);
void uart0_interrupt_get_string(unsigned char* buffer, unsigned long length, unsigned long* ptr, bool* string_complete);
//bool get_char(uint8_t *c);
//void send_char(uint8_t c);


extern struct ring_buffer rx_ring_buffer;
extern struct ring_buffer tx_ring_buffer;

#endif	// UART_INTERRUPT_H
