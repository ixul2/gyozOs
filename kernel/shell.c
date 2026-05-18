#include "shell.h"

int is_shifted = 0;
volatile int key_ready = 0;
char last_key = 0;

void keyboard_handler(unsigned char c){
    uint16_t *VGABuffer = (uint16_t*) 0xB8000;
    char ascii_code = keyboard_to_ascii(c, is_shifted);
    if ((c == 0x2A) || (c == 0x36)){
    	is_shifted += 1;
    }
    else if ((c == 0xAA) || (c == 0xB6)){
    	is_shifted -= 1;
    }
    if(ascii_code){ 
        key_ready = 1;
        last_key = ascii_code;
    }
}

void shell_print_char(int cursor, char c){
    uint16_t *VGABuffer = (uint16_t*) 0xB8000;
    VGABuffer[cursor] = 0x0F00 | c;
}

int shell(){
	while(1);
}
