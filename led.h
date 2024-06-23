#ifndef LED_H
#define LED_H

#include "tm4c123gh6pm.h"    // Device header

//	*** Global Variables

// Global switch variables to read in from peripherals on PA0 and PA4
// These are DECLARED as "extern" in the header file, then DEFINED in the files
//	they will be used in
extern unsigned long Switch_1;
extern unsigned long Switch_2;

//	*** Function Prototypes
void port_f_initialization(void);
void led_switches(void);
void led_toggle(void);

#endif	// LED_H
