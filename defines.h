// My own header file for register memory mappings for practice
// 	instead of using the CMSIS provided header file


// *** Constant Declarations

// System Control Registers
#define SYSCTL_RCGCGPIO_R		(*((volatile unsigned long *) 0x400FE608))
#define SYSCTL_RCGCUART_R		(*((volatile unsigned long *) 0x400FE618))

// Usage of symbolic names instead of numbers to make it more readable
//
// -- GPIO Offsets --
// Found in the datasheet
// DATA: 	0x000 + Adding 3FC because we use bits [9:2], bits [1:0] are 0
// DIR:  	0x400
// AFSEL: 0x420
// AMSEL: 0x528
// DEN: 	0x51C
// PCTL: 	0x52C
// PUR:		0x510
// LOCK: 	0x520
// CR:		0x524
//
// GPIO Port F
// Base Address: 0x40025000
#define GPIO_PORTF_DATA_R 	(*((volatile unsigned long *) 0x400253FC))
#define GPIO_PORTF_DIR_R		(*((volatile unsigned long *) 0x40025400))
#define GPIO_PORTF_AFSEL_R	(*((volatile unsigned long *) 0x40025420))
#define GPIO_PORTF_AMSEL_R	(*((volatile unsigned long *) 0x40025528))
#define GPIO_PORTF_DEN_R		(*((volatile unsigned long *) 0x4002551C))
#define GPIO_PORTF_PCTL_R		(*((volatile unsigned long *) 0x4002552C))
#define GPIO_PORTF_PUR_R 		(*((volatile unsigned long *) 0x40025510))
#define GPIO_PORTF_LOCK_R 	(*((volatile unsigned long *) 0x40025520))
#define GPIO_PORTF_CR_R 		(*((volatile unsigned long *) 0x40025524))
	
// GPIO Port A
// Base Address: 0x40004000
#define GPIO_PORTA_AFSEL_R	(*((volatile unsigned long *) 0x40004420))
#define GPIO_PORTA_AMSEL_R	(*((volatile unsigned long *) 0x40004528))
#define GPIO_PORTA_DEN_R		(*((volatile unsigned long *) 0x4000451C))
#define GPIO_PORTA_PCTL_R		(*((volatile unsigned long *) 0x4000452C))


// -- UART Offsets --
// Found in the datasheet
// IBRD: 	0x024
// FBRD:	0x028
// LCRH:	0x02C
// CTL:		0x030
// CC: 		0xFC8
// FR:		0x018
// DATA:	0x000
//
// UART 0
// Base Address: 0x4000C000
#define UART0_IBRD_R 	(*((volatile unsigned long *) 0x4000C024))
#define UART0_FBRD_R 	(*((volatile unsigned long *) 0x4000C028))
#define UART0_LCRH_R 	(*((volatile unsigned long *) 0x4000C02C))
#define UART0_CTL_R 	(*((volatile unsigned long *) 0x4000C030))
#define UART0_CC_R 		(*((volatile unsigned long *) 0x4000CFC8))
#define UART0_FR_R 		(*((volatile unsigned long *) 0x4000C018))
#define UART0_DR_R 		(*((volatile unsigned long *) 0x4000C000))

