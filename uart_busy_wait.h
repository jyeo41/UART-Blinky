// *** PURPOSE
// uart_busy_wait was a stepping stone uart driver file to make sure the uart port 0
// was working properly because busy-wait is easy to implement. The goal was to use this
// to get a better understanding of serial transmission and then implement interrupts to improve
// the efficiency of the code.

#ifndef UART_BUSY_WAIT_H
#define UART_BUSY_WAIT_H

#include "tm4c123gh6pm.h"    // Device header

//	*** Function Prototypes

void uart0_busy_wait_initialization(void);
char uart0_busy_wait_read_char(void);
void uart0_busy_wait_write_char(char data);
void uart0_busy_wait_write_string(char* string);
void uart0_busy_wait_read_string(char* buffer, unsigned long length);
void uart0_busy_wait_menu(char* buffer, unsigned long length);
unsigned long strings_compare_case_insensitive(const char* string_1, const char* string_2);

#endif	// UART_BUSY_WAIT_H
