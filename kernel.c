//PAH PAH PAH ON FAIT UN KERNEL
#include <stdint.h>

int print();

int kernel(){
    uint16_t* video = (uint16_t*)0xB8000;
	print();
    const char* msg = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    uint8_t color = 0x0F; // White on black

    for (int i = 0; i < 1; i++) {
    	char c = msg[54];
        int a = ((uint16_t)color << 8) | (c%10)+48;
    }

    while (1); // Halt
}

int print(){
	return 2;
}
