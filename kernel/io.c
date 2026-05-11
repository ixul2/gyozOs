#include <stdint.h>
#include "io.h"

uint16_t *VGABuffer = (uint16_t*) 0xB8000;

char lowercase_caracters_row0[] = "&e\"'(-e_ca)=";
char lowercase_caracters_row1[] = "azertyuiop^$";
char lowercase_caracters_row2[] = "qsdfghjklmù*";
char lowercase_caracters_row3[] = "wxcvbn,;:!";

char uppercase_caracters_row0[] = "1234567890°+";
char uppercase_caracters_row1[] = "AZERTYUIOP¨£";
char uppercase_caracters_row2[] = "QSDFGHJKLM%µ";
char uppercase_caracters_row3[] = "WXCVBN?./§";

char keyboard_to_ascii(unsigned char c, int is_shifted){
    if ((0x02 <= c) && (c <= 0x0B)){
    	if (is_shifted)
    		return uppercase_caracters_row0[c-02];
    		
        return lowercase_caracters_row0[c-02];
    }
    else if ((0x10 <= c) && (c <= 0x1B)){
    	if (is_shifted)
    		return uppercase_caracters_row1[c-0x10];
    		
        return lowercase_caracters_row1[c-0x10];
    } 
    else if ((0x1E <= c) && (c <= 0x27)){
    	if (is_shifted)
    		return uppercase_caracters_row2[c-0x1E];
    		
        return lowercase_caracters_row2[c-0x1E];
    } 
    else if ((0x2C <= c) && (c <= 0x33)){
    	if (is_shifted)
    		return lowercase_caracters_row3[c-0x2C];
   
        return uppercase_caracters_row3[c-0x2C];
    }
    else if (c == 0x39){
    	return ' ';
    }
    else{
        return 0;
    }
}

void cleanScreen(){
	for (int i = 0; i < 80*25; i++){
		VGABuffer[i] = 0x0F00;
	}
}

void console_print(uint32_t l, uint32_t c, const char* s){
    volatile uint16_t* vmem = (uint16_t*) 0xB8000;
    uint32_t max = 80 * 25;
    uint32_t pos = l*80 + c;
    for(uint32_t i = 0; s[i] != '\0'; i++){
        vmem[(pos + i) % max] = s[i] | 0x0F00 ;
    }
}

void console_print_int(uint32_t l, uint32_t c, unsigned int n){
	char buffer[64];
	buffer[63] = '\0';
	int cursor = 62;
	while(n || (cursor == 62)){
		buffer[cursor--] = 48+(n%10);
		n = n/10;
	}
	console_print(l, c, &buffer[cursor+1]);
}

void console_print_int_wrapper(unsigned int n){
	console_print_int(0, 0, n);
}

void fail(const char* errorMsg){
	cleanScreen();
	console_print(0, 0, errorMsg);
	while(1);
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void serial_write_char(char c) {
    outb(0x3F8, c);
}

void serial_write(const char* s) {
    while (*s)
        serial_write_char(*s++);
}

void serial_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

void assert(int b, const char* msg){
    if(b == 0){
        fail(msg);
    }
}