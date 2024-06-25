#include "uart_busy_wait.h"
//#include "defines.h"

#define NULL 0
#define TRUE 1
#define FALSE 0


// Changed UART0_CC_R from 0 (system clock) to 0x5 (PIOSC Precision Internal Oscillator)and it finally got the UART transmission to work properly!!
// Presumably the system clock is not default set to 16 MHz clock frequency? I'm not exactly sure why that is.
//
// TODO
// Look into why setting the system clock did not allow the UART to function properly.
void uart0_busy_wait_initialization(void)
{
	unsigned long delay;
	SYSCTL_RCGCUART_R |= 0x01;		// Enable clock gating for UART0
	delay = SYSCTL_RCGCUART_R;		// Let the clock settle
	SYSCTL_RCGCGPIO_R |= 0x01;		// Enable clock gating for Port A
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTA_AFSEL_R = 0x03;		// Alternate function enabled because UART
	GPIO_PORTA_PCTL_R |= 0x11;		// 4 bit wide PMCs, setting PMC0 and PMC1 according to page 1351
	GPIO_PORTA_DEN_R |= 0x03;			// Enable digital on PA1-0
	GPIO_PORTA_AMSEL_R &= ~0x03u;	// Disable analog on PA1-0
	
	// Baud Rate calculation:
	// BRD = Bus clock / 16 * input baud rate
	// We assume 16 MHz base clock, input baud rate of 9600 on Port A UART
	// BRD = 16000000 / (16 * 9600) = 104.166666666666...7
	// IBRD = 104
	// FBRD = integer(0.166666...7 * 64 + 0.5) = integer(11.16666...7) = 11
	UART0_CTL_R &= ~0x01u;				// Disable UART by clearing the UARTEN bit, bit0
	UART0_IBRD_R = 104;						// Set the IBRD
	UART0_FBRD_R = 11;						// Set the FBRD
	UART0_LCRH_R = 0x70;					// 8 bit, no parity, FIFO enabled, one stop bit
	UART0_CC_R = 0x5;							// Clock source to be set to internal oscillator, 16 MHz, setting it to 0 default system clock doesn't work
	UART0_CTL_R |= 0x01; 					// Enable UART by setting the UARTEN bit 0, bit [9:8] for transmit/receive
	
}

// Function to set the LEDs on the MCU depending on the string input from the user
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
void uart0_busy_wait_menu(char* buffer, unsigned long length)
{
	uart0_busy_wait_write_string("Enter one of the following colors:\n");
	uart0_busy_wait_write_string("Red, Blue, Green, Pink, Yellow, Cyan, White, Black\n");
	uart0_busy_wait_read_string(buffer, length);
	uart0_busy_wait_write_string("Received: ");
	uart0_busy_wait_write_string(buffer);
	uart0_busy_wait_write_string("\n\n");
	
	if(strings_compare_case_insensitive(buffer, "red") == 0)
	{
		GPIO_PORTF_DATA_R = 0x02;
	}
	else if(strings_compare_case_insensitive(buffer, "blue") == 0)
	{
		GPIO_PORTF_DATA_R = 0x04;
	}
	else if(strings_compare_case_insensitive(buffer, "green") == 0)
	{
		GPIO_PORTF_DATA_R = 0x08;
	}
	else if(strings_compare_case_insensitive(buffer, "pink") == 0)
	{
		GPIO_PORTF_DATA_R = 0x06;
	}
	else if(strings_compare_case_insensitive(buffer, "yellow") == 0)
	{
		GPIO_PORTF_DATA_R = 0x0A;
	}
	else if(strings_compare_case_insensitive(buffer, "cyan") == 0)
	{
		GPIO_PORTF_DATA_R = 0x0C;
	}
	else if(strings_compare_case_insensitive(buffer, "white") == 0)
	{
		GPIO_PORTF_DATA_R = 0x0E;
	}
	else
	{
		GPIO_PORTF_DATA_R = 0x00;
	}
}

char uart0_busy_wait_read_char(void)
{
	// Keep polling until the UART Receiver FIFO is not empty.
	// This means there is data to be consumed.
	// 0 means receive FIFO is not empty
	// 1 means receive FIFO IS empty
	while((UART0_FR_R & 0x10) != 0);
	return UART0_DR_R & 0xFF;
}

void uart0_busy_wait_write_char(char c)
{
	// some terminals expect carriage return '\r' before line-feed '\n' for proper new line.
	// this seems to happen on putty where if you don't have it, it'll print the characters
	// on the terminal in a diagonal fashion
	if(c == '\n')
	{
		uart0_busy_wait_write_char('\r');
	}
	
	// Keep polling until the UART Transmit FIFO is not full.
	// This means there is data to be sent.
	while ((UART0_FR_R & 0x20) != 0);
	UART0_DR_R = c;
}

// NOTE:
// Need to use && check in if(c != '\n' && c != '\r') because || would always evaluate to true.
// if c == '\n' then c != '\r' would be true so the statement would be true with || and vice versa.
// if c is any other character, it would also evalulate to true, therefore the condition is always true.
void uart0_busy_wait_read_string(char* buffer, unsigned long length)
{
	unsigned long i = 0;	// used to iterate through the incoming buffer array
	unsigned char c;			// used to read in data from the receive FIFO
	unsigned long exit_flag = 0;

	while(exit_flag == FALSE)
	{
		c = uart0_busy_wait_read_char();	// read in the character
		uart0_busy_wait_write_char(c);		// display the character immediately
		
		// IMPORTANT check for 'DEL' or '\b' character so the buffer doesn't get flooded with backspace characters
		// and destroy the string written in the buffer. Decrementing the buffer index also "deletes" the character
		// in the buffer as well, just like in the terminal.
		if(c == 0x7F)		
		{
			if(i > 0)
			{
				i--;
			}
		}
		else if(c != '\n' && c != '\r')	// if its a character, keep writing the string into the buffer
		{
			buffer[i++] = c;
			if(i == (length - 1))			// if we're at the last index of the buffer, put a null terminator
			{
				buffer[i] = '\0';
				exit_flag = 1;
			}
		}
		else	// if the input character is a '\n' or '\r'
		{
			buffer[i] = '\0';
			exit_flag = 1;
		}
	}
}

void uart0_busy_wait_write_string(char* string)
{
	if (string == NULL)
	{
		return;
	}
	
	// print the string a character at a time until it reaches the null terminator '\0'
	while(*string)
	{
		uart0_busy_wait_write_char(*string);
		string++;
	}
}


// Function to compare case insenstiive strings
// This is so if the user types RED or Red or red, it shouldn't matter when trying to choose the LED color
//
// BUG: When using the backspace key on the terminal, it puts that ASCII character into the buffer.
// 			This results in the string compare function failing.
//			i.e. grww BS BS een, which results in green on the terminal
//				however the buffer would look like grww@@een where the @'s are just placeholders for ASCII backspace
unsigned long strings_compare_case_insensitive(const char* string_1, const char* string_2)
{
	char char_1, char_2;
	
	// Keep comparing the strings as long as they're not NULL
	while(*string_1 && *string_2)
	{
		// Converting uppercase characters to lower case characters
		if(*string_1 >= 'A' && *string_1 <= 'Z')
		{
			char_1 = *string_1 - 'A' + 'a';
		}
		else
		{
			char_1 = *string_1;
		}
		
		if(*string_2 >= 'A' && *string_2 <= 'Z')
		{
			char_2 = *string_2 - 'A' + 'a';
		}
		else
		{
			char_2 = *string_2;
		}
		
		// If at any point the characters are a mismatch
		//	return 1 to say they are not equal
		if(char_1 != char_2)
		{
			// busy_wait_write_string("Strings are NOT equal.\n");
			return 1;
		}
		
		// increment the pointers to keep the check going
		string_1++;
		string_2++;
	}
	// if both strings start with the same string but are of different lengths,
	//	this also means they are not equal, so return 1
	//	i.e. "hello" and "helloworld"
	if(*string_1 || *string_2)
	{
		// busy_wait_write_string("Strings are NOT equal.\n");
		return 1;
	}
	
	// if all of the checks pass, then they are the same string so return 0
	// busy_wait_write_string("Strings are equal.\n");
	return 0;
}
