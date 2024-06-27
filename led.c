#include "led.h"
#include "test.h"
#include "systick.h"

// Port F Initialization Function
// The steps to initialize were followed through using the datasheet section 10.3
// Key notes:
// Using bit friendly setting to only touch the relevant bits
// PF4 and PF0 require the internal pull up resistor to be on because
//	the schematic shows it is only connected to GND and floating the VCC 
// PF0 is also locked by default, so we unlock it in the LOCK_R and then CR_R
void port_f_initialization(void)
{
	SYSCTL_RCGCGPIO_R |= 0x20u;				// enable clock gating for port f
	systick_wait_5ms(1);
	GPIO_PORTF_DIR_R |= 0x0Eu;				// PF4 PF0 to input 0, PF3-1 to output 1
	GPIO_PORTF_AFSEL_R &= ~0x1Fu;			// Disable alternative function
	GPIO_PORTF_PCTL_R &= ~0x1Fu;			// Clear bits to set as GPIO
	GPIO_PORTF_PUR_R |= 0x11u;				// Enable pull up resistors for PF4 and PF0
	GPIO_PORTF_AMSEL_R &= ~0x1Fu;			// Disable analog
	GPIO_PORTF_LOCK_R = 0x4C4F434Bu;	// Writing this specific value to the register to unlock
	GPIO_PORTF_CR_R	|= 0x01u;					// Allow changes to PF0
	GPIO_PORTF_DEN_R |= 0x1Fu;				// Enable digital for all pins	
}

// Function to read in Switch 1 and Switch 2 and turn on the board LED
//
// Color    LED(s) PortF
// dark     ----    0x00
// red      --R-    0x02
// blue     -B--    0x04
// green    G---    0x08
// pink			-BR-    0x06
// yellow		G-R-    0x0A
// cyan			GB--    0x0C
// white		GBR-    0x0E
unsigned long Switch_1;
unsigned long Switch_2;
void led_switches(void)
{
		// Negative logic switches, 0 means pushed and 1 means not pushed
		Switch_1 = (GPIO_PORTF_DATA_R & 0x10) >> 4;		// extracting the input bit from PF4 SW1
		Switch_2 = (GPIO_PORTF_DATA_R & 0x01) >> 0;		// extracting the input bit from PF0 SW2
		
		if(Switch_1 == 0 && Switch_2 == 0)
		{
			GPIO_PORTF_DATA_R = 0x08;		// if both are pressed, LED is green
		}
		else
		{
			if(Switch_1 == 0 && Switch_2 == 1)
			{
				GPIO_PORTF_DATA_R = 0x04;	// Only switch 1, LED is blue
			}
			else
			{
				if(Switch_1 == 1 && Switch_2 == 0)
				{
					GPIO_PORTF_DATA_R = 0x02;	// Only switch 2, LED is red
				}
				else
				{
					GPIO_PORTF_DATA_R = 0x00;	// No switches are pressed, turn off LED
				}
			}
		}
}

void led_toggle(void)
{
	GPIO_PORTF_DATA_R ^= 0x02;
	delay(1000000);
}
