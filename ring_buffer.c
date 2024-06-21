#include "ring_buffer.h"

void ring_buffer_initialization(struct ring_buffer* rb, unsigned long* buffer, unsigned long buffer_size)
{
	rb->buffer = buffer;
	rb->buffer_size = buffer_size;
	rb->counter = 0;
	rb->head = 0;
	rb->tail = 0;
}

void ring_buffer_write(struct ring_buffer* rb, unsigned long data)
{
	rb->buffer[rb->head] = data;									// Set data to the current head index since it is the next available index
  rb->counter += 1;
	rb->head = (rb->head + 1) % rb->buffer_size;	// Increment head by 1 and use % operator to loop around after reaching the end
}

unsigned long ring_buffer_read(struct ring_buffer* rb)
{
	unsigned long data = rb->buffer[rb->tail];		// Extract data from the current tail position
  rb->counter -= 1;
	rb->tail = (rb->tail + 1) % rb->buffer_size;	// Increment tail afterwards, using % operator to loop around after the end
	return data;
}

bool ring_buffer_is_empty(struct ring_buffer* rb)
{
	return rb->counter == 0;	// Buffer is full when both are equal. When initializing, head and tail are both = 0.
										// If you write 2 elements then read 2 elements, again head and tail are both equal and its empty.
}

bool ring_buffer_is_full(struct ring_buffer* rb)
{
	return rb->counter == rb->buffer_size; // The buffer is full if adding one more element would make head and tail equal.
}

