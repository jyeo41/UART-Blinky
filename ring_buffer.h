#include <stdbool.h>

// Ring buffer implementation to be used with UART interrupts

struct ring_buffer {
	unsigned long *buffer;			// buffer array to hold the data in the ring buffer
	unsigned long buffer_size;	// buffer array size
	unsigned long head;					// head index
	unsigned long tail;					// tail index
	unsigned long counter;
};

void ring_buffer_initialization(struct ring_buffer *rb, unsigned long* buffer, unsigned long buffer_size);
void ring_buffer_write(struct ring_buffer *rb, unsigned long data);
unsigned long ring_buffer_read(struct ring_buffer *rb);
bool ring_buffer_is_empty(struct ring_buffer *rb);
bool ring_buffer_is_full(struct ring_buffer *rb);
