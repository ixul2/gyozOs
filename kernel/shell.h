#ifndef _SHELL_H_
#define _SHELL_H_

#include "io.h"
#include "system_library.h"
#include "lib.h"

void keyboard_handler(unsigned char c);
void wait_for_input(int);
void shell_print_char(char);
void shell_print(char*);
int shell();
#endif