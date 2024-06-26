#include <stdint.h>
#include <stdbool.h>
#include "led.h"
#include "uart_busy_wait.h"
#include "uart_interrupt.h"
#include "systick.h"
#include "test.h"

int main(void)
{
	//char buffer[100];
	unsigned char color[100];			// static buffer to hold color strings such as "red", "blue", etc
	unsigned long color_ptr = 0;	// pointer to keep track of the color buffer to "build" the string properly
	bool string_complete = false;	// flag to check if string was completely built after user hit enter, used to reset color_ptr
	systick_initialization();
	port_f_initialization();
	uart0_interrupt_initialization();
	// Global interrupts enabled by default.

	// main loop
	while(1)
	{
		uart0_interrupt_get_string(color, 100, &color_ptr, &string_complete);
	}
}


/***** Project Notes *****
// defines.h
Start with the defines.h to map out the registers I would need for each module by using the datasheet.
This was just to practice understanding memory mapped registers. Eventually switch over to the given vendor header file.

Problem: Mistyped one of the memory mappings and couldn't figure out if it was a problem with the bit setting, or the #define
Solution: Had to double check and painstakingly check the datasheet to make sure the offsets were correct.
					In the future, use the included header files from the vendors to save user errors.


// led.c and led.h
Test the GPIO pins and make sure the board LED and switches were functioning properly.
I also made sure the Port F Initialization was done properly and to confirm the register mappings in my defines.h were working.

Problem: Ran into a problem with PF0 switch not functioning even though the bits were set correctly.
Solution: Page 650.Table 10-1 shows PF[0] is locked by default and needs to be unlocked and enabled with the GPIO_LOCK registers GPIO_CR


// uart_busy_wait.c and uart_busy_wait.h
Implement UART functionality between PC and TM4C123GXL MCU. The goal was to be able to type a user input string from the terminal to
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
					
					
// uart_interrupt.c and uart_interrupt.h
Improve the code to be more efficient using interrupts rather than busy wait.

Problem: Lots of trouble setting up UART0 interrupts to trigger properly.
Solution: Turns out the vector table in the header file is incorrect.
					It is shown as 21 as well as 21 in the datasheet, however when I inspected the CMSIS files, it showed the UART0 interrupt as interrupt request 5.
					After modifying the NVIC_EN and NVIC_PRI appropriately to account for IRQ 5 instead of IRQ 21, the ISR triggered successfully.

Problem: Transmit interrupt was not triggering even though all the UART initializations were correct and transmit interrupt being enabled in the UART_IM register.
Solution: Have to directly write a byte into the UART_DR once when the TX FIFO is empty to initialize the transmission 
					and have the hardware detect and flag the TXRIS bit in RIS register. The bit is actually checked in the MIS register for programming logic.
					This was the hardest problem to solve because of a combination of weird debugger behaviors, registers not updating while stepping through code (maybe an IDE issue?)
					and the data sheet being incorrect. The data sheet validity being the biggest offender.
					When the FIFOs are disabled through clearing the FEN bit in UART_LCRH register, the hardware FIFOs are set to a depth of 1. The datasheet says, when the TX FIFO is empty
					if FEN bit is cleared, this means its ready to transmit and trigger the transmit interrupt TXRIS. This was false and you still have to write the byte into the UART_DR.
					I suspect its because when the FIFOs are enabled with a depth of 16, it uses "through level" triggering of the TXRIS. This means you need to keep writing a number of elements
					until the TX FIFO reaches a certain % filled i.e. 1/8, 1/2, etc. When the through level is hit, it triggers the TXRIS.
					With a depth of 1, its supposed to trigger each time it is empty, but I think it uses "through level" triggering as well so you have to write the first byte. 
					Took me many many hours to figure out this problem. I just never suspected the data sheet could be that incorrect.
					
Problem: Weird debugger behaviors.
Solution:	When trying to debug the UART ISR, you HAVE to set a breakpoint inside of the interrupt flag check if statement to confirm it, otherwise if you place the breakpoints
					anywhere else, such as the beginning of the ISR, the interrupt RXRIS and transmit TXRIS trigger bits don't ever set for some reason.
					Breakpoint debugging seems to tamper with ISR behavior when stepping through code. It was much easier to just toggle LEDs and test one if statement at a time while
					letting the code run with no breakpoints to check if the ISR was triggering. The debugger i'm using is the Stellaris ICDI thats on the launchpad. 
					I suspect the weird ISR behavior is because UART0 is directly connected to the debugger. The ICDI uses UART0 for serial communication but now that I have
					configured it for my program's use, its interfering with it.
*/
