// #include "defines.h"
#include "led.h"
// #include "uart_busy_wait.h"
#include "uart_interrupt.h"
#include "test.h"
#include <stdint.h>


#define CIRC_BUF_LEN 12
static uint8_t rx_buf[CIRC_BUF_LEN];
static uint8_t tx_buf[CIRC_BUF_LEN];
static uint16_t rx_get_ptr = 0;
static uint16_t rx_put_ptr = 0;
static uint16_t tx_get_ptr = 0;
static uint16_t tx_put_ptr = 0;

void send_char(uint8_t c);
bool get_char(uint8_t *c);
// void start_transmission(uint8_t c);

int main(void)
{
	// char buffer[100];
	unsigned char data;
	// int counter = 0;
	port_f_initialization();
	delay(1000000);
	uart0_interrupt_initialization();
	delay(1000000);
	// Global interrupts enabled by default.
	// __enable_irq();

	// main loop
	while(1)
	{
		
		//uart0_interrupt_send_char(&tx_ring_buffer, 'a');
		//uart0_interrupt_send_char(&tx_ring_buffer, 'b');
		
		// Check the status of the rx_ring_buffer in a non-blocking way.
		// If the receive ISR triggered and put data into the rx_ring_buffer,
		//	send the char to tx_ring_buffer and enable the transmit interrupt.
//		if(uart0_interrupt_get_char(&rx_ring_buffer, &data))
//		{
//			uart0_interrupt_send_char(&tx_ring_buffer, data);
//		}
		if (get_char(&data))
		{
				send_char(data);
		}
//		if (counter == 1000000)
//		{
//			UART0_DR_R = 'Z';
//		}
//		
//		data = 'T';
//		send_char(data);
//		counter++;
		
	}
}

void send_char(uint8_t c) {
		// If no current transmission is going (Tx hardware FIFO is empty)then start it by writing directly to the UART0_DR_R register
		if (UART0_FR_R & 0x80)
		{
			UART0_DR_R = c;
			uart0_interrupt_enable_transmit();
		}
		// If transmission is on going, put it in the tx buffer
		else
		{
			tx_buf[tx_put_ptr++] = c;
			tx_put_ptr &= (CIRC_BUF_LEN - 1);
		}
}

bool get_char(uint8_t *c) {
    if (rx_get_ptr == rx_put_ptr)
		{
        return false;			
		}
    *c = rx_buf[rx_get_ptr++];
    rx_get_ptr &= (CIRC_BUF_LEN - 1);

    return true;
}

void UART0_Handler(void) {
	GPIO_PORTF_DATA_R = 0x04;		// set blue LED to confirm ISR is triggered
	
	if (UART0_MIS_R & 0x10) { //&& (UART0_FR_R & 0x40)) {
		GPIO_PORTF_DATA_R = 0x02;		// set red LED to confirm it entered the receive interrupt if statement
		rx_buf[rx_put_ptr++] = UART0_DR_R & 0xFF;
		rx_put_ptr &= (CIRC_BUF_LEN - 1);
	}

	if (UART0_MIS_R & 0x20) { //&& (UART0_FR_R & 0x80)) {
		GPIO_PORTF_DATA_R ^= 0x08;		// set green LED to confirm it entered the transmit interrupt if statement
		if (tx_get_ptr != tx_put_ptr) {
				UART0_DR_R = tx_buf[tx_get_ptr++];
				tx_get_ptr &= (CIRC_BUF_LEN - 1);
		} else {
				uart0_interrupt_disable_transmit();
		}
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


*/
