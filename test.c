#include "test.h"
#include "ring_buffer.h"

// Test functions

// Simple delay function just to quickly test certain functions
void delay(unsigned long delay)
{
	unsigned long i;
	for (i = 0; i < delay; i++);
}

void test_ring_buffer(void)
{
	struct ring_buffer rb;
	unsigned long buffer[5];
	unsigned long data;
	
	ring_buffer_initialization(&rb, buffer, 5);
	
	
}