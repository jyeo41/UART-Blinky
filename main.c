#include "defines.h"
#include "led.h"
#include "uart.h"
#include "TM4C123.h"                    // Device header

void delay(unsigned long delay);


int main(void)
{
	port_f_initialization();
	uart_0_initialization();
	char buffer[100];
	
	// main loop
	while(1)
	{
		uart_busy_wait_menu(buffer, 100);
	}
}


// Simple delay function just to quickly test certain functions
void delay(unsigned long delay)
{
	unsigned long i;
	for (i = 0; i < delay; i++);
}

