#include <stdint.h>
int print_int(uint64_t n){
	__asm__ volatile(
    "mov %0, %%rdi\n"
    "int $48\n"
    : 
    : "r"(n)        // no input
    : "rdi"         // clobber RDI if modified
	);
}
