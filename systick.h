#ifndef SYSTICK_H
#define SYSTICK_H
#include "tm4c123gh6pm.h"		// vendor provided header
#include <stdint.h>

void systick_initialization(void);
void systick_wait_reload(uint32_t reload);
void systick_wait_5ms(uint32_t iterations);
void systick_test(void);

#endif 	// SYSTICK_H
