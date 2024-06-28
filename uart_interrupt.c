#include "uart_interrupt.h"
#include "systick.h"

#define NULL 0
#define TRUE 1
#define FALSE 0

/***** High Level Interrupts Concept *****/
// The idea is, enable interrupts in both the peripheral/device, NVIC for that peripherial/device, and global
//
// 1) Enable the clock gating for specific peripheral, in this case UART0 and GPIO PORTA
// 2) Configure the peripheral and additional configurations to allow interrupts, UARTIM register for UART
// 3) Enable NVIC interrupt
// 4) Enable NVIC Priority
// 5) Enable global interrupts in PRIMASK_R, set I bit to 0


/***** Interrupts - Software and Hardware Flow *****/
// So the big picture idea for this program is: 
// I as the programmer ENABLE receive interrupts in the software, inside the UART Initialization function using the IM register.
// This allows the MCU hardware to ACTIVATE the receive interrupt and set the flag in the MIS register when I actually send a character over putty.
// The ISR handles the hardware triggered interrupt, and the software CLEARS THE ACTIVE flag in MIS register by writing to the ICR register. 
// This still means the flag is ENABLED in the IM register so the receive interrupt can keep triggering over and over as long as the condition is met.
//
// On the transmit side, the software also needs to enable the transmit interrupt in IM register. 
// The hardware ACTIVATES the transmit interrupt in the MIS register. 
// The software CLEARS the active interrupt flag in the MIS register by writing to the ICR register 
// 	and also DISABLES the transmit interrupt entirely by clearing the bit in IM until it should be enabled again by the receiver.
// 
// What im now understanding is:
// 1) the software ARMS the interrupt in IM Register
// 2) hardware TRIGGERS the interrupt and sets it ACTIVE in the MIS register
// 3) software ACKNOWLEDGES and CLEARS the active interrupt in the ICR register
// 4) Steps 2 and 3 keep repeating until the software decides to DISARM the interrupt in the IM register again

/***** Debugging UART 0 Memory Address *****/
// UART 0 Base Address: 0x4000 C000
// LCRH:	0x02C
// IM:		0x038
// RIS:		0x03C
// MIS:		0x040
// FR:		0x018

#define UART_BUFFER_SIZE (150)	// Need a big enough buffer size to be able to handle overflow
static unsigned long rx_buffer[UART_BUFFER_SIZE];
static unsigned long tx_buffer[UART_BUFFER_SIZE];

struct ring_buffer rx_ring_buffer = 
{
	.buffer = rx_buffer,
	.buffer_size = UART_BUFFER_SIZE,
	.head = 0,
	.tail = 0,
	.counter = 0
};

struct ring_buffer tx_ring_buffer = 
{
	.buffer = tx_buffer,
	.buffer_size = UART_BUFFER_SIZE,
	.head = 0,
	.tail = 0,
	.counter = 0
};

// The Interrupt Vector Table is located on page 104 of the datasheet
void uart0_interrupt_initialization(void)
{
	SYSCTL_RCGCUART_R |= 0x01;		// Enable clock gating for UART0
	systick_wait_5ms(1);
	SYSCTL_RCGCGPIO_R |= 0x01;		// Enable clock gating for Port A
	systick_wait_5ms(1);
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
	UART0_LCRH_R = 0x60;					// 8 bit, no parity, FIFO disabled, one stop bit
	UART0_CC_R = 0x5;							// Clock source to be set to internal oscillator, 16 MHz, setting it to 0 default system clock doesn't work
	UART0_CTL_R |= 0x301; 				// Enable UART by setting the UARTEN bit 0, bit [9:8] for transmit/receive, Enable Tx and Rx bits
	
	// Enable interrupts for UART specific
	UART0_IM_R |= 0x10;																			// Enable receive interrupts, bit 4
  NVIC_EN0_R |= 1 << 5;           												// Enable IRQ 5 (UART0), tried it with IRQ21 and didn't work but setting it to 5 does work
  NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFF00FF) | (2 << 13); 	// Set priority to 2
}

// Function to enable transmit interrupt
void uart0_interrupt_enable_transmit(void)
{
	UART0_IM_R |= 0x20;
}

// Function to disable transmit interrupt
void uart0_interrupt_disable_transmit(void)
{
	UART0_IM_R &= ~0x20u;
}

void uart0_interrupt_clear_receive(void)
{
	UART0_ICR_R |= 0x10;
}

void uart0_interrupt_clear_transmit(void)
{
	UART0_ICR_R |= 0x20;
}

// UART0 interrupt service routine
// Technically don't need to clear interrupts because datasheet specifies they auto clear
//	when reading from or writing to the UART_DR. It's still good practice to do so 
void UART0_Handler(void)
{
	//GPIO_PORTF_DATA_R = 0x04;		// set blue LED to confirm receive interrupt triggered
	
	// Receive interrupt triggered
	// If receive interrupt bit 4 is set in the MIS register, put the received data into the rx_ring_buffer
	if (UART0_MIS_R & 0x10)
	{
		// GPIO_PORTF_DATA_R = 0x02;		// set red LED to confirm it entered the receive interrupt if statement
		
		// Acknowledge receive interrupt
		uart0_interrupt_clear_receive();
		
		// Keep reading from the hardware receive FIFO as long as its not empty and
		//	receiver ring buffer is not full
		while (!(UART0_FR_R & 0x10) && !ring_buffer_is_full(&rx_ring_buffer))
		{
			// Read the data out from the HW RxFIFO and put it into the rx_ring_buffer
			ring_buffer_write(&rx_ring_buffer, UART0_DR_R);
		}

	}
	
	// Transmit interrupt triggered
	// If transmit interrupt bit 5 is set in the MIS register
	if (UART0_MIS_R & 0x20)
	{
		// GPIO_PORTF_DATA_R = 0x08;		// set green LED to confirm transmit interrupt triggered
		
		// Acknowledge transmit receive interrupt
		// uart0_interrupt_clear_transmit();
		
		// Check to make sure the tx_ring_buffer has data to be transmitted
		while ((UART0_FR_R & 0x80) && !ring_buffer_is_empty(&tx_ring_buffer))
		{
			UART0_DR_R = ring_buffer_read(&tx_ring_buffer);						
		}

		// If theres no data in the tx_ring_buffer to be sent then disable the interrupt.
		// The transmit interrupt should never trigger when there's no data to be transmitted
		if (ring_buffer_is_empty(&tx_ring_buffer))
		{
			uart0_interrupt_disable_transmit();
		}
	}
}

// Function to get a single char from the rx_ring_buffer after the ISR has put data into it.
// It returns a boolean to call uart0_interrupt_send_char() function afterwards if it was successful.
// This is because the send_char() function should only write data to tx_ring_buffer if data was received.
//	The transmit interrupt should also only be enabled when the tx_ring_buffer has data in it.
//
// Ended up being a test function instead to check the ISR was correctly placing received bytes into the rx_ring_buffer
bool uart0_interrupt_get_char(struct ring_buffer* rb, unsigned char* c)
{
	if (ring_buffer_is_empty(rb))
	{
		return false;
	}
	else
	{
		*c = ring_buffer_read(rb) & 0xFF;
	}
	return true;
}


// Function to place the char from the rx_ring_buffer into the tx_ring_buffer.
//	Only when there is data in the tx_ring_buffer should transmit interrupts be enabled
//
// FEN bit in UART_LCRH_R set to 0 means FIFO disabled with a depth of 1
// FEN bit in UART_LCRH_R set to 1 means FIFO enabled with a depth 16. 
// The interrupt's trigger conditions change depending on the FEN bit.
// Datasheet pages 900-901
void uart0_interrupt_send_char(struct ring_buffer* rb, unsigned char c)
{
	uart0_interrupt_disable_transmit();
	// some terminals expect carriage return '\r' before line-feed '\n' for proper new line.
	// this seems to happen on putty where if you don't have it, it'll print the characters
	// on the terminal in a diagonal fashion
	if(c == '\n')
	{
		ring_buffer_write(rb, '\r');
	}
	ring_buffer_write(rb, c);
	
	// Place char to be echoed into the transmit ring buffer

	
	// If the transmit FIFO is empty, initiate the transmission by writing a byte directly
	//	into the transmit FIFO by reading it from the tx_ring_buffer.
	// This is necessary because for some reason, when the FIFO depth is 1, the datasheet incorrectly states
	//	the TXRIS flag will trigger when the TxFIFO is empty. After extensive testing, in order to trigger the TXRIS flag
	//	you still need to write the initial byte into the FIFO to initiate the transmission.
	if (UART0_FR_R & 0x80)
	{
		UART0_DR_R = ring_buffer_read(rb);
	}

	// Enable transmit interrupts
	uart0_interrupt_enable_transmit();
}

// Driver level function to properly extract the color string i.e. "red", "blue", "green", etc the user types into to be later
//	called by an application level function to check the color. Decided to separate out the functions compared to the big menu
//	function in the uart_busy_wait.c file for easier testing and improved code structure
//
// Variables:
// static buffer = the buffer used to store the correctly extracted color string
// length = the length of the static buffer
// ptr = index of the static buffer to correctly "build" the string using index logic
//	to account for '\n', '\r', and most important the 'DEL' or backspace
// string_complete = flag to check whether the string has been successfully built after user hits enter. This function is called
//	non-blocking without using a loop, so it needs to know when to reset the buffer ptr back to 0 so the buffer doesn't hold
// 	multiple colors inside of it and rebuilds the string starting from the 0 index
void uart0_interrupt_get_string(char* static_buffer, unsigned long length, unsigned long* ptr, bool* string_complete)
{
	unsigned char c;			// used to read in character from rx_ring_buffer

	// This whole block should only initiate when there is actual data inside of the rx_ring_buffer.
	// This means the ISR handled the receiver transmission of data from the user and placed it in the rx_ring_buffer.
	if(!ring_buffer_is_empty(&rx_ring_buffer))
	{
		c = ring_buffer_read(&rx_ring_buffer) & 0xFF;			// read in the character
		uart0_interrupt_send_char(&tx_ring_buffer, c);		// display the character immediately
		
		// If its a backspace character, in Putty it is sent as 'DEL' which is 0x7F,
		//	decrement static buffer index so the string isn't destroyed by holding the junk character
		if (c == 0x7F)
		{
			if (*ptr > 0)
			{
				(*ptr)--;
			}
		}
		else if ((c != '\n') && (c != '\r'))	// If its a real character add it to the static buffer while checking overflow
		{
			
			if (*ptr < length - 1)	// As long as buffer is not full, put the character in we should continue to build the string
			{
				static_buffer[(*ptr)++] = c;
				*string_complete = false;
			}
			else		// If we reached the final index, force null terminate it
			{
				static_buffer[*ptr] = '\0';
				*string_complete = true;
			}
		}
		else		// If it's a '\r' or '\n' character, terminate the string because the user finished typing it
		{
			static_buffer[*ptr] = '\0';
			*string_complete = true;
		}
	}
}

// Function to send strings over UART
void uart0_interrupt_send_string(const char* string)
{	
	// Keep looping until the '\0'
	while (*string)
	{
		// Some terminals expect carriage return '\r' before line-feed '\n' for proper new line.
		// This seems to happen on putty where if you don't have it, it'll print the characters
		// 	on the terminal in a diagonal fashion
		if(*string == '\n')
		{
			ring_buffer_write(&tx_ring_buffer, '\r');
		}
		
		// If the TX FIFO is empty AND the ISR doesn't "own" the data register, start the transmission by writing a byte to the UART_DR_R.
		if (!(UART0_IM_R & 0x20))	// if transmission is not enabled
		{
			if (!(UART0_FR_R & 0x20))	// if the TX FIFO is NOT full, in other words it has space
			{
				UART0_DR_R = *string;	// we should write the char directly into the data register and enable the transmission
				uart0_interrupt_enable_transmit();
			}
			else	// if the TXFIFO is indeed full, we should write the char to the tx ring buffer instead
			{
				ring_buffer_write(&tx_ring_buffer, *string);
			}
		}
		else	// If the transmission is enabled, then the ISR "owns" UART_DR_R, we have to treat it like its utilizing it. Write to the tx ring buffer to not interfere.
		{
			ring_buffer_write(&tx_ring_buffer, *string);			
		}
		string++;
	}
}
