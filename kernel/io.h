#ifndef _IO_H_
#define _IO_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void console_print(uint32_t l, uint32_t c, const char* s);
void console_print_int(uint32_t l, uint32_t c, unsigned int n);
void console_print_int_wrapper(unsigned int n);
char keyboard_to_ascii(unsigned char c, int is_shifted);
void cleanScreen();
void fail(const char* errorMsg);

void serial_write_char(char c);
void serial_write(const char* s);
void serial_init(void);
void assert(int b, const char* msg);

#endif