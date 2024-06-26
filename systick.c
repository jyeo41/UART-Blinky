#include "systick.h"

// Use systick_wait() functions set at different delays to let the clocks stabilize in initialization functions
//	i.e. uart0_interrupt_initialization() and port_f_initialization()

#define SYSTICK_RELOAD_MAX 0x00FFFFFF		// Max is 24 bits

void systick_initialization(void)
{
	NVIC_ST_CTRL_R = 0;			// disable systick while initializing it
	NVIC_ST_CTRL_R = 0x01;	// Set bit 0 to enable it, clear bit 2 to disable systick interrupts
													// clear bit 3 to set CLK_SRC as PIOSC
}

void systick_wait_reload(uint32_t reload)
{
	// If the value is > the max, then we should set it to max as a check
	if (reload > SYSTICK_RELOAD_MAX)
	{
		reload = SYSTICK_RELOAD_MAX;
	}
	
	// The -1 because the COUNT bit in ST_CTRL_R triggers when CURRENT goes from 1 to 0. We START counting from delay.
	// I.e. if the RELOAD value is 10, it would count like so: 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0. Starting FROM 10, and counting TO 0 is 11 counts.
	// 	To make sure the reload value is calculated correctly, we need to -1. 
	NVIC_ST_RELOAD_R = reload - 1;
	NVIC_ST_CURRENT_R = 0;	// Set current to 0 to set the COUNT bit 16 in NVIC_ST_CTRL_R
	
	// Keep delaying until the COUNT bit is set.
	while ((NVIC_ST_CTRL_R & 0x00010000) == 0);
}

// Used to delay after setting clocks and letting them set in UART and GPIO Port initialization functions
// Assume 16 MHz clock using PIOSC
//
// Calculations: COUNT of 80k in systick_wait_5ms(80000)
// Formula is delay = count * clock tick
// desired delay = 5ms
// clock tick = 62.5ns
//	1/F where Frequency is 16MHz
// count = 5ms/62.5ns = 80000
void systick_wait_5ms(uint32_t iterations)
{
	for (uint32_t i = 0; i < iterations; i++)
	{
		systick_wait_reload(80000);
	}
}
