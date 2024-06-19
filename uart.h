//	*** Function Prototypes

void uart_0_initialization(void);
char busy_wait_read_char(void);
void busy_wait_write_char(char data);
void busy_wait_write_string(char* string);
void busy_wait_read_string(char* buffer, unsigned long length);
void uart_busy_wait_menu(char* buffer, unsigned long length);
unsigned long strings_compare_case_insensitive(const char* string_1, const char* string_2);
