#ifndef TEST_H
#define TEST_H

#include "tm4c123gh6pm.h"    // Device header
#include "ring_buffer.h"
#include "systick.h"
#include "led.h"

void delay(unsigned long delay);
void test_ring_buffer(void);
#endif	// TEST_H
