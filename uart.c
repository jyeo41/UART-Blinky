#include "uart.h"
#include "defines.h"

#define NULL 0


// Changed UART0_CC_R from 0 (system clock) to 0x5 (PIOSC Precision Internal Oscillator)and it finally got the UART transmission to work properly!!
// Presumably the system clock is not default set to 16 MHz clock frequency? I'm not exactly sure why that is.
//
// TODO
// Look into why setting the system clock did not allow the UART to function properly.
void uart_0_initialization(void)
{
	unsigned long delay;
	SYSCTL_RCGCUART_R |= 0x01;		// Enable clock gating for UART0
	delay = SYSCTL_RCGCUART_R;		// Let the clock settle
	SYSCTL_RCGCGPIO_R |= 0x01;		// Enable clock gating for Port A
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTA_AFSEL_R = 0x03;		// Alternate function enabled because UART
	GPIO_PORTA_PCTL_R |= 0x11;		// 4 bit wide PMCs, setting PMC0 and PMC1 according to page 1351
	GPIO_PORTA_DEN_R |= 0x03;			// Enable digital on PA1-0
	GPIO_PORTA_AMSEL_R &= ~0x03u;	// Disable analog on PA1-0
	
	// Baud Rate calculation:
	// BRD = Bus clock / 16 * input baud rate
	// We assume 16 MHz base clock, input baud rate of 9600 on Port A UART
	// BRD = 16000000 / (16 * 9600) = 104.166666666666...7
	// IBRD = 104
	// FBRD = integer(0.166666...7 * 64 + 0.5) = integer(11.16666...7) = 11
	UART0_CTL_R &= ~0x01u;				// Disable UART by clearing the UARTEN bit, bit0
	UART0_IBRD_R = 104;						// Set the IBRD
	UART0_FBRD_R = 11;						// Set the FBRD
	UART0_LCRH_R = 0x70;					// 8 bit, no parity, FIFO enabled, one stop bit
	UART0_CC_R = 0x5;							// Clock source to be set to internal oscillator, 16 MHz, setting it to 0 default system clock doesn't work
	UART0_CTL_R |= 0x01; 					// Enable UART by setting the UARTEN bit 0, bit [9:8] for transmit/receive
	
}

char busy_wait_read_char(void)
{
	while((UART0_FR_R & 0x10) != 0);
	return UART0_DR_R & 0xFF;
}

void busy_wait_write_char(char c)
{
	// some terminals expect carriage return '\r' before line-feed '\n' for proper new line.
	// this seems to happen on putty where if you don't have it, it'll print the characters
	// on the terminal in a diagonal fashion
	if (c == '\n')
	{
		busy_wait_write_char('\r');
	}
	while ((UART0_FR_R & 0x20) != 0);
	UART0_DR_R = c;
}

void busy_wait_read_string(char* buffer, unsigned long length)
{
	unsigned long i = 0;	// used to iterate through the incoming buffer array
	unsigned char c;	// used to read in data from the receive FIFO

	while(i < length - 1)
	{
		c = busy_wait_read_char();	// read in the character using busy wait
		busy_wait_write_char(c);
		if(c == '\n' || c == '\r')	// if the user has finished typing the string, terminate the string in the buffer
		{
			buffer[i] = '\0';
			return;
		}
		buffer[i++] = c;	// keep writing the string
	}
	buffer[i] = '\0';	// if we reach the last available index in the buffer, put a null terminator
}

void busy_wait_write_string(char* string)
{
	if (string == NULL)
	{
		return;
	}
	// read in the string a character at a time until it reaches the null terminator '\0'
	while(*string)
	{
		busy_wait_write_char(*(string++));
	}
}

void busy_wait_read_string_flag(char* buffer, unsigned long length)
{
	unsigned long i = 0;	// used to iterate through the incoming buffer array
	unsigned char c;	// used to read in data from the receive FIFO

	while(i < length - 1)
	{
		c = busy_wait_read_char();	// read in the character using busy wait
		busy_wait_write_char(c);
		if(c == '\n' || c == '\r')	// if the user has finished typing the string, terminate the string in the buffer
		{
			buffer[i] = '\0';
			return;
		}
		buffer[i++] = c;	// keep writing the string
	}
	buffer[i] = '\0';	// if we reach the last available index in the buffer, put a null terminator
}