#ifndef TEST_H
#define TEST_H

#include "tm4c123gh6pm.h"    // Device header
#include "ring_buffer.h"
#include "systick.h"
#include "led.h"
#include "colors.h"

void delay(unsigned long delay);
void test_ring_buffer(void);
void test_systick_wait(void);
void test_print_request_color(void);
#endif	// TEST_H
