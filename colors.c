#include "colors.h"


Colors get_color(char* static_buffer, unsigned long length, unsigned long* ptr, volatile bool* string_complete)
{
	// Check if we are building a new string.
	if (*string_complete)
	{
		*string_complete = false;
		*ptr = 0;			// If the user completed typing a string in the previous iteration, then we should reset the pointer
									// 	to build the string properly on the next iteration starting from the beginning of the buffer.
									// This line of code was refactored from the uart_interrupt0_get_string() function due to the programming logic.
	}

	// Continue getting the string until it's complete, i.e. the user hits enter.
	while (!(*string_complete))
	{
		uart0_interrupt_get_string(static_buffer, length, ptr, string_complete);
	}
	
	// Return the corresponding color from the Colors enum.
	if (strings_compare_colors_case_insensitive(static_buffer, "red"))
	{
		return RED;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "blue"))
	{
		return BLUE;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "green"))
	{
		return GREEN;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "yellow"))
	{
		return YELLOW;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "pink"))
	{
		return PINK;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "cyan"))
	{
		return CYAN;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "white"))
	{
		return WHITE;
	}
	else if (strings_compare_colors_case_insensitive(static_buffer, "black"))
	{
		return BLACK;
	}
	else
	{
		return NO_COLOR;
	}
}

// Simple switch statement on the incoming enum value to turn on the corresponding LED color.
// If no valid color was entered, then it should also turn the LED off.
void led_turn_on_color(Colors led_color)
{
	switch (led_color)
	{
		case RED:
			GPIO_PORTF_DATA_R = 0x02;
			break;
		
		case BLUE:
			GPIO_PORTF_DATA_R = 0x04;
			break;
		
		case GREEN:
			GPIO_PORTF_DATA_R = 0x08;
			break;
		
		case YELLOW:
			GPIO_PORTF_DATA_R = 0x0A;
			break;
		
		case PINK:
			GPIO_PORTF_DATA_R = 0x06;
			break;
		
		case CYAN:
			GPIO_PORTF_DATA_R = 0x0C;
			break;
		
		case WHITE:
			GPIO_PORTF_DATA_R = 0x0E;
			break;
		
		case BLACK: 
			GPIO_PORTF_DATA_R = 0x00;
			break;
		
		case NO_COLOR:
			GPIO_PORTF_DATA_R = 0x00;
			break;
	}
}

// Function to compare case insenstiive strings.
// This is so if the user types RED or Red or red, it shouldn't matter when trying to choose the LED color.
//
// BUG: When using the backspace key on the terminal, it puts that ASCII character into the buffer.
// 			This results in the string compare function failing.
//			i.e. grww BS BS een, which results in green on the terminal
//				however the buffer would look like grww@@een where the @'s are just placeholders for ASCII backspace
bool strings_compare_colors_case_insensitive(const char* string_1, const char* string_2)
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
			return false;
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
		return false;
	}
	
	// if all of the checks pass, then they are the same string so return 0
	// busy_wait_write_string("Strings are equal.\n");
	return true;
}

void print_request_color(const char* string)
{
//	const char* color_string = string;
//	if (color == NO_COLOR)
//	{
//		color_string = "Error: Invalid color\r\n"; 
//	}
	while (*string)
	{
		uart0_interrupt_send_char(&tx_ring_buffer, *string);
		string++;
	}
}


void clear_line(void)
{
	uart0_interrupt_send_char(&tx_ring_buffer, '\r');      // Move cursor to start of line
	for (uint8_t i = 0; i < 100; i++)
	{
		uart0_interrupt_send_char(&tx_ring_buffer, ' ');
	}
	
	uart0_interrupt_send_char(&tx_ring_buffer, '\r');      // Move cursor to start of line again
}
