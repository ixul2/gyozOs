#ifndef _SHELL_H_
#define _SHELL_H_

#include "io.h"
#include "system_library.h"
#include "lib.h"
#include "x86-64.h"
#include "process.h"

#define KB_BUFFER_SIZE 16

void keyboard_handler(void);
void wait_for_input(int);
void shell_print_char(int, char);
int shell();

#endif