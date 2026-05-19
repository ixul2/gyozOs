#include "shell.h"

/* Global state for the keyboard driver */
static int extended = 0;
static int is_shifted = 0;

/* Simple ring buffer for characters (including arrow‑key codes) */

char kb_buffer[KB_BUFFER_SIZE];
int kb_head = 0;
int kb_tail = 0;

/* Wakeup mechanism – the process waiting for keyboard input */
proc *keyboard_waiting = 0;

/* Helper: add a character to the buffer and wake a blocked process */
static void kb_buffer_put(char c) {
    int next = (kb_head + 1) % KB_BUFFER_SIZE;
    if (next != kb_tail) {           // not full
        kb_buffer[kb_head] = c;
        kb_head = next;
    }
}

/* New keyboard handler – called from generic_exception_handler */
void keyboard_handler() {
    /* 1. Read the scan code directly from hardware */
    unsigned char c = inb(0x60);

    /* 2. Process the scan code (same logic as before) */
    char ascii_code = keyboard_to_ascii(c, is_shifted);

    if (c == 0xE0) {
        extended = 1;
    } else if ((c == 0x2A) || (c == 0x36)) {
        is_shifted += 1;
    } else if ((c == 0xAA) || (c == 0xB6)) {
        is_shifted -= 1;
    } else if (extended && c == 0x4B) {
        kb_buffer_put(1);  // left arrow (map to 1 for your shell)
    } else if (extended && c == 0x4D) {
        kb_buffer_put(2);  // right arrow (map to 2)
    } else if (extended && c == 0x48) {
        kb_buffer_put(3);  // up arrow (map to 3)
    } else if (ascii_code) {
        kb_buffer_put(ascii_code);
    }

    if (c != 0xE0)
        extended = 0;   // reset extended flag unless it's a two‑byte prefix

    /* 3. Signal End‑of‑Interrupt to the master PIC */
    outb(0x20, 0x20);

    if(keyboard_waiting && kb_head != kb_tail){
        keyboard_waiting->reg.reg_rax = kb_buffer[kb_tail];
        kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
        keyboard_waiting->state = P_RUNNABLE;
        schedule();
    }
}

void shell_print_char(int cursor, char c){
    uint16_t *VGABuffer = (uint16_t*) 0xB8000;
    VGABuffer[cursor] = 0x0F00 | c;
}

int shell(){
	while(1);
}
