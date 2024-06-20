#include "defines.h"
#include "led.h"
#include "uart.h"
#include "TM4C123.h"                    // Device header

void delay(unsigned long delay);


int main(void)
{
	char buffer[100];
	port_f_initialization();
	uart_0_initialization();
	delay(1000000);
	
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

/***** Project Notes *****
// defines.h
Started with the defines.h to map out the registers I would need for each module by using the datasheet.
This was just to practice understanding memory mapped registers.

Problem: Mistyped one of the memory mappings and couldn't figure out if it was a problem with the bit setting, or the #define
Solution: Had to double check and painstakingly check the datasheet to make sure the offsets were correct.
					In the future, use the included header files from the vendors to save user errors.


// led.c and led.h
Next the led.c and .h files to test the GPIO pins and make sure the board LED and switches were functioning properly.
I also made sure the Port F Initialization was done properly and to confirm the register mappings in my defines.h were working.

Problem: Ran into a problem with PF0 switch not functioning even though the bits were set correctly.
Solution: Page 650.Table 10-1 shows PF[0] is locked by default and needs to be unlocked and enabled with the GPIO_LOCK registers GPIO_CR


// uart.c and uart.h
Implemented UART functionality between PC and TM4C123GXL MCU. The goal was to be able to type a user input string from the terminal to
switch on one of the 8 possible LED colors on the board. Ran into a lot of problems during this and was a learning experience.

Problem: Junk characters being passed in the terminal when typing out characters.
Solution: Turns out setting UART0_CC_R = 0 was a mistake. This sets it to the default system clock which is supposed to run at 16MHz.
					Had to change the setting to UART0_CC_R = 0x5 which sets it to the PIOSC precision internal oscillator which is 16MHz as well.
					Fixed the baudrate issue and solved junk characters being shown on the terminal.
					
Problem: When typing characters into the terminal, it would print in a diagonal line.
Solution: Some terminals need a carriage return '\r' before a '\n' for it to print properly. The following if statement solved the issue.
					if(c == '\n')
					{
						busy_wait_write_char('\r');
					}

Problem: Tried to debug with the simulator but since UART0 is configured and its connected to the debugger, it blocked being able to test the code in simulation.
Solution: Set breakpoints in the functions I needed to and had to debug looking at the variables in the 'watch' window and debug directly using the hardware. Not ideal.

Problem: Implemented own strcmp() function without the use of external libraries but realized case sensitivity was an issue when trying to switch the LEDs.
Solution: Edited the strcmp() to be a string_compare_case_insensitive() function.

Problem: Implementing string transmit and receive (read and write) functions to switch the LED color. Would sometimes fail even when inputting the correct string.
Solution: The buffer to store the user input string also stores the backspace character. This resulted in deleting characters on the terminal
					but still printing out the received color normally. The UART write function would print "blue" when in reality it was "blu[][]e" where [] are backspace/deletes.
					Weirdly enough Putty receives the backspace as 0x7F which is the ASCII control character for 'DEL'. Tried to configure Putty settings to read in
					backspaces as '\b' 0x08 instead of 'DEL' 0x7F so I could add in a condition to decrement the index of the buffer. This resulted in the terminal not deleting characters
					at all when using the backspace key.
					Edited the uart_read_string() function to have an additional check for the 0x7F ('DEL') to decrement the index of the buffer so the string would
					be proper when string compare checking for the switch statement. This handles deleting any characters, including spaces. LED always functions properly now.
*/
