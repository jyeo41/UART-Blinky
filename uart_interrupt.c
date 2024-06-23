#include "uart_interrupt.h"

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


#define UART_BUFFER_SIZE (16)
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
	SYSCTL_RCGCGPIO_R |= 0x01;		// Enable clock gating for Port A
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
	UART0_IM_R |= 0x10;						// Enable receive interrupts bit 4
	// UART0_IM_R |= 0x20;						// Enable transmit interrupts bit 4
																													
  NVIC_EN0_R |= 1 << 5;           												// Enable IRQ 5 (UART0), tried it with IRQ21 and didn't work but setting it to 5 does work
  NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFF00FF) | (2 << 13); 	// Set priority to 2
	
	// NVIC_EN0_R |= (1 << 21);															// UART0 is interrupt 21, NVIC_EN0 is for interrupts 0-31, PRI5 is for 20-23
	// NVIC_PRI5_R = (NVIC_PRI5_R & 0xFFFF0FFF) | 0x4000;		// Masking with 0xFFFF0FFF first to clear any previously set priority, and not touch other bits
																													// 0x4000 puts 010 in bits [15:13] interrupt priority
	//NVIC_EnableIRQ(UART0_IRQn);
	//NVIC_SetPriority(UART0_IRQn, 2);
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
//void UART0_Handler(void)
//{
//	volatile unsigned long txim = UART0_MIS_R & 0x20;	// sanity checking interrupt trigger flag
//	volatile unsigned long rxim = UART0_MIS_R & 0x10;	// sanity checking interrupt trigger flag
//	GPIO_PORTF_DATA_R = 0x04;		// set blue LED to confirm ISR is triggered
//	
//	// Receive interrupt triggered
//	// If bit is set in the MIS register AND Receive FIFO is full (data has been received and is ready to be read),
//	// 	put the received data into the rx_ring_buffer
////	if ((UART0_MIS_R & 0x10) && (UART0_FR_R & 0x40))
////	{
////		GPIO_PORTF_DATA_R = 0x02;		// set red LED to confirm it entered the receive interrupt if statement
////		UART0_DR_R = 'R';						// show 'R' on terminal too
////		// Acknowledge receive interrupt
////		uart0_interrupt_clear_receive();
////		
////		// Read the data out from the HW RxFIFO and put it into the rx_ring_buffer
////		// ring_buffer_write(&rx_ring_buffer, UART0_DR_R);
////	}
//	// Transmit interrupt triggered
//	// If interrupt bit is set in the MIS register AND Transmit FIFO is empty (UART is ready to send more data)
//	if ((UART0_MIS_R & 0x20) && (UART0_FR_R & 0x80))
//	{
//		GPIO_PORTF_DATA_R = 0x08;		// set green LED to confirm it entered the transmit interrupt if statement
//		UART0_DR_R = 'G';						// show 'G' on terminal too
//		// Acknowledge transmit receive interrupt
//		uart0_interrupt_clear_transmit();
//		
//		// Also check to make sure the tx_ring_buffer has data to be transmitted
//		if (!ring_buffer_is_empty(&tx_ring_buffer))
//		{
//			UART0_DR_R = ring_buffer_read(&tx_ring_buffer);
//		}
//		// if it doesn't, then disable the interrupt because if theres no data in the tx_ring_buffer to be sent,
//		//	the transmit interrupt should never trigger
//		else
//		{
//			uart0_interrupt_disable_transmit();
//		}
//	}
//}

// Function to get a single char from the rx_ring_buffer after the ISR has put data into it.
// It returns a boolean to call uart0_interrupt_send_char() function afterwards if it was successful.
// This is because the send_char() function should only write data to tx_ring_buffer if data was received.
//	The transmit interrupt should also only be enabled when the tx_ring_buffer has data in it.
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
void uart0_interrupt_send_char(struct ring_buffer* rb, unsigned char c)
{
	ring_buffer_write(rb, c);
	uart0_interrupt_enable_transmit();
}
