# UART Blinky Project
This repository tracks the code for a blinky project that connects a microcontroller to the PC to turn on
different colored LEDS on the launchpad. The user can type in one of the 8 possible colors from the RGB LED
through a serial terminal (such as Putty).

Extensive and detailed comments have been documented throughout the project. In the bottom of main.c, there is a write up of the more difficult issues I faced while building this project from scratch and the solutions I discovered through debugging.

Testing and debugging was done through Stellaris ICDI, using LEDs throughout functions, and stepping through code using breakpoints and the debugger. Some test functions were also implemented inside of the test.c file.

# Build
IDE: Keil Microvision5
MCU: TM4C123 (TM4C123GXL specifically)
Terminal: Putty
