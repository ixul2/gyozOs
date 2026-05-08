#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

void console_print(uint32_t l, uint32_t c, const char* s);
char keyboard_to_ascii(unsigned char c, int is_shifted);
