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

	}
}

