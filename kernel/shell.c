#include "io.h"
#include "system_library.h"

uint16_t *VGABuffer = (uint16_t*) 0xB8000;
int cursor = 0;
int is_shifted = 0; 

void cleanScreen(){
	for (int i = 0; i < 80*25; i++){
		VGABuffer[i] = 0x0F00;
	}
}

void keyboard_handler(unsigned char c){
    char ascii_code = keyboard_to_ascii(c, is_shifted);
    if ((c == 0x2A) || (c == 0x36)){
    	is_shifted += 1;
    }
    else if ((c == 0xAA) || (c == 0xB6)){
    	is_shifted -= 1;
    }
    else if (c == 0x1C){
    	cursor = cursor + 80 - (cursor%80);
    }
    else if (ascii_code){
        VGABuffer[cursor++] = 0x0F00+ascii_code;
    }
}

int shell(){
	cleanScreen();
	print_int(10);
	while(1);
}
