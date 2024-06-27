#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"    // Device header
#include "uart_interrupt.h"
#include "led.h"

// Enum used to return the color code to flash the correct LED color
typedef enum Colors
{
	NO_COLOR,
	RED,
	BLUE,
	GREEN,
	YELLOW,
	PINK,
	CYAN,
	WHITE,
	BLACK
} Colors;

Colors get_color(char* static_buffer, unsigned long length, unsigned long* ptr, volatile bool* string_complete);
void led_turn_on_color(Colors led_color);
bool strings_compare_colors_case_insensitive(const char* string_1, const char* string_2);
void print_request_color(const char* string);
void clear_line(void);
#endif	// COLORS_H

