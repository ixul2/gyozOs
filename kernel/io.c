#include <stdint.h>

int is_shifted = 0; 

char lowercase_caracters_row0[] = "&é\"'(-è_çà)=";
char lowercase_caracters_row1[] = "azertyuiop^$";
char lowercase_caracters_row2[] = "qsdfghjklmù*";
char lowercase_caracters_row3[] = "wxcvbn,;:!";

char uppercase_caracters_row0[] = "1234567890°+";
char uppercase_caracters_row1[] = "AZERTYUIOP¨£";
char uppercase_caracters_row2[] = "QSDFGHJKLM%µ";
char uppercase_caracters_row3[] = "wxcvbn?./§";

char keyboard_to_ascii(unsigned char c){
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
    else{
        return 0;
    }
}

void keyboard_handler(unsigned char c){
    uint16_t *VGABuffer = (uint16_t*) 0xB8000;
    if ((c == 0x2A) || (c == 0x36)){
    	is_shifted += 1;
    }
    if ((c == 0xAA) || (c == 0xB6)){
    	is_shifted -= 1;
    }
    char ascii_code = keyboard_to_ascii(c);
    if (ascii_code)
        VGABuffer[0] = 0x0F00+ascii_code;
}

