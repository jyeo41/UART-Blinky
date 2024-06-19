//	*** Function Prototypes

void uart_0_initialization(void);
char busy_wait_read_char(void);
void busy_wait_write_char(char data);
void busy_wait_write_string(char* string);
void busy_wait_read_string(char* buffer, unsigned long length);
void busy_wait_read_string_flag(char* buffer, unsigned long length);


