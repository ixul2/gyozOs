#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void console_print(uint32_t l, uint32_t c, const char* s);
void console_print_int(uint32_t l, uint32_t c, unsigned int n);
void console_print_int_wrapper(unsigned int n);
char keyboard_to_ascii(unsigned char c, int is_shifted);
void cleanScreen();
void fail(char* errorMsg);

void *memset(void *dst, int c, size_t n);
