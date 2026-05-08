#include <stdint.h>
#include "io.h"



char lowercase_caracters_row0[] = "&e\"'(-e_ca)=";
char lowercase_caracters_row1[] = "azertyuiop^$";
char lowercase_caracters_row2[] = "qsdfghjklmù*";
char lowercase_caracters_row3[] = "wxcvbn,;:!";

char uppercase_caracters_row0[] = "1234567890°+";
char uppercase_caracters_row1[] = "AZERTYUIOP¨£";
char uppercase_caracters_row2[] = "QSDFGHJKLM%µ";
char uppercase_caracters_row3[] = "wxcvbn?./§";

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
   
        return lowercase_caracters_row3[c-0x2C];
    }
    else if (c == 0x39){
    	return ' ';
    }
    else{
        return 0;
    }
}

void console_print(uint32_t l, uint32_t c, const char* s){
    volatile uint16_t* vmem = (uint16_t*) 0xB8000;
    uint32_t max = 80 * 25;
    uint32_t pos = l*80 + c;
    for(uint32_t i = 0; s[i] != '\0'; i++){
        vmem[(pos + i) % max] = s[i] | 0x0F00 ;
    }
    return;
}
