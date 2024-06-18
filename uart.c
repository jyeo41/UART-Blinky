#include "uart.h"
#include "defines.h"

void uart_0_initialization(void)
{
	unsigned long delay;
	SYSCTL_RCGCUART_R |= 0x01;	// Enable clock gating for UART0
	delay = SYSCTL_RCGCUART_R;	// Let the clock settle
	SYSCTL_RCGCGPIO_R |= 0x01;	// Enable clock gating for Port A
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTA_AFSEL_R = 0x03;	// Alternate function enabled because UART
	GPIO_PORTA_PCTL_R |= 0x11;	// 4 bit wide PMCs, setting PMC0 and PMC1 according to page 1351
	GPIO_PORTA_DEN_R	|= 0x03;	// Enable digital on PA1-0
	GPIO_PORTA_AMSEL_R &= ~0x03u;	// Disable analog on PA1-0
	
	// Baud Rate calculation:
	// BRD = Bus clock / 16 * input baud rate
	// We assume 16 MHz base clock, input baud rate of 9600 on Port A UART
	// BRD = 16000000 / (16 * 9600) = 104.166666666666...7
	// IBRD = 104
	// FBRD = integer(0.166666...7 * 64 + 0.5) = integer(11.16666...7) = 11
	UART0_CTL_R &= ~0x01u;	// Disable UART by clearing the UARTEN bit, bit0
	UART0_IBRD_R = 104;			// Set the IBRD
	UART0_FBRD_R = 11;			// Set the FBRD
	// UART0_LCRH_R = 0x60;		// 8 bit, no parity, FIFO disabled, one stop bit
	UART0_LCRH_R = 0x70;		// 8 bit, no parity, FIFO enabled, one stop bit
	UART0_CC_R = 0;					// Clock source to be system clock
	UART0_CTL_R |= 0x01; 		// Enable UART by setting the UARTEN bit 0, bit [9:8] for transmit/receive
	
}
