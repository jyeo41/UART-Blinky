#include "defines.h"
#include "led.h"
#include "uart.h"
#include "TM4C123.h"                    // Device header

void delay(unsigned long delay);

int main(void)
{
	// unsigned char c;
	char buffer[100];
	port_f_initialization();
	uart_0_initialization();
	
	// main loop
	while(1)
	{
//		// Wait until the user inputs a character then print it on the Putty terminal
//		c = busy_wait_read_char();
//		busy_wait_write_char(c);
//		
//		// show next character on new line in putty terminal
//		busy_wait_write_char('\n');
		busy_wait_read_string(buffer, 100);
		busy_wait_write_string("Received: ");
		busy_wait_write_string(buffer);
		busy_wait_write_char('\n');
		
//		switch(c)
//		{
//			case 'r':
//				GPIO_PORTF_DATA_R = 0x02;
//				break;
//			
//			default:
//				GPIO_PORTF_DATA_R &= ~0x0Eu;
//				break;
//		}
	}
}


// Simple delay function just to quickly test certain functions
void delay(unsigned long delay)
{
	unsigned long i;
	for (i = 0; i < delay; i++);
}

