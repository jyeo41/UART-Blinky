#include "test.h"

// Test functions

// Simple delay function just to quickly test certain functions
// Replaced by systick delay function
void delay(unsigned long delay)
{
	unsigned long i;
	for (i = 0; i < delay; i++);
}


// Simple test function to check the ring buffer is working properly
// Checking read and write implementations, as well as checking if its full and empty
void test_ring_buffer(void)
{
	struct ring_buffer rb;
	unsigned long buffer[5];
	unsigned long data;
	unsigned long i;
	
	ring_buffer_initialization(&rb, buffer, 5);
	
	// Fill up the ring buffer to test for normal capacity
	for (i = 0; i < rb.buffer_size + 1; i++)
	{
		data = i + 3;
		
		// Flash green LED on successful writes, flash blue LED if buffer is full to test for overflow checking
		// Two delays to give user enough time to see the flashing of the LED
		if (!ring_buffer_is_full(&rb))
		{
			ring_buffer_write(&rb, data);
			GPIO_PORTF_DATA_R = 0x08;
		}
		else
		{
			GPIO_PORTF_DATA_R = 0x04;
		}
		delay(1000000);
		GPIO_PORTF_DATA_R = 0x00;
		delay(1000000);
	}
	
	for (i = 0; i < rb.buffer_size + 1; i++)
	{		
		// Flash yellow LED on successful reads, flash red LED if buffer is full to test for underflow checking
		// Two delays to give user enough time to see the flashing of the LED
		if (!ring_buffer_is_empty(&rb))
		{
			data = ring_buffer_read(&rb);
			GPIO_PORTF_DATA_R = 0x0A;
		}
		else
		{
			GPIO_PORTF_DATA_R = 0x02;
		}
		delay(1000000);
		GPIO_PORTF_DATA_R = 0x00;
		delay(1000000);
	}
}

// Test function to check whether the systick wait is functioning properly.
void test_systick_wait(void)
{
	GPIO_PORTF_DATA_R ^= 0x02;
	systick_wait_5ms(1);
}

void test_uart0_interrupt_send_string(void)
{
	uart0_interrupt_send_string("Enter one of the following colors:\n");
	uart0_interrupt_send_string("Red, Blue, Green, Pink, Yellow, Cyan, White, Black\n");
	systick_wait_5ms(50);
}
