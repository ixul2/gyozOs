#include <stdint.h>

uint32_t print();

uint32_t kernel(){
    volatile uint16_t* vmem = (uint16_t*) 0xB8000;
    for (uint32_t i = 0; i < 400; i++){
    vmem[i] = 0x1F41+(i%26);
    }
    while (1); // Halt
}

uint32_t print(){
	return 2;
}
