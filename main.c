#include "defines.h"
#include "led.h"
#include "uart.h"
#include "TM4C123.h"                    // Device header


int main(void)
{
	unsigned char c;
	port_f_initialization();
	uart_0_initialization();
	
	// main loop
	while(1)
	{
		led_switches();
		print_string("Enter \"r\", \"g\", or \"b\":\n\r");
		
		c = read_char();
		print_char(c);
		print_string("\n\r");
		
		switch(c)
		{
			case 'r':
				GPIO_PORTF_DATA_R = 0x02;
				break;
			
			default:
				GPIO_PORTF_DATA_R = 0;
		}
	}
}

