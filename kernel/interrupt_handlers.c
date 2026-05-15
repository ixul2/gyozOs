#include "interrupt_handlers.h"

extern void dummy_handler(void);
extern void keyboard_handler_wrapper(void);
extern void pagefault_handler_wrapper(void);
extern void syscall_handler_wrapper(void);
extern void print_int_asm(void);

IDT_entry IDTable[256];

void setupIDTEntry(IDT_entry* IDTEntry, uint64_t handlerAddr, int privilege){
    IDTEntry->offset_low  = handlerAddr & 0xFFFF;
    IDTEntry->selector    = 0x08; //use kernel segment from the GDT
    IDTEntry->ist         = 0; //use current stack
    IDTEntry->zero1       = 0;
    IDTEntry->type_attr   = 0x8E + (privilege << 5);
    IDTEntry->offset_mid  = (handlerAddr >> 16) & 0xFFFF;
    IDTEntry->offset_high = (handlerAddr >> 32) & 0xFFFFFFFF;
    IDTEntry->zero2       = 0;
}

void setupIDTable(IDT_ptr *idt_ptr){
	setupIDTEntry(&IDTable[14], (uint64_t) pagefault_handler_wrapper, 0); //pagefault
	setupIDTEntry(&IDTable[33], (uint64_t) keyboard_handler_wrapper, 0); //keyboard
	setupIDTEntry(&IDTable[48], (uint64_t) print_int_asm, 3); //custom interrupts
	setupIDTEntry(&IDTable[0x80], (uint64_t) syscall_handler_wrapper, 3); // system calls
	idt_ptr->base = (uint64_t) IDTable;
	idt_ptr->limit = sizeof(IDTable)-1;
}

void setupPIC(){
	outb(0x20, 0x11); //send ICW1 to slave and master (initialization)
	outb(0xA0, 0x11);
	
	outb(0x21, 0x20); //IRQ0 -> INT 0x20 ... IRQ7 -> INT 0x27 (ICW2)	
	outb(0x20, 0x28); //IRQ8 -> INT 0x28 ... IRQ15 -> INT 0x2F
	
	
	outb(0x21, 0x04); //send ICW3 to connect slave to IRQ2
	outb(0xA1, 0x02);
	
	outb(0x21, 0x01); //enable 8086/88 mode
	outb(0xA1, 0x01);
	
	outb(0x21, 0xFD); //unmask keyboard IRQ
	outb(0xA1, 0xFF);
}

void setupInterrupts(){
	IDT_ptr idt_ptr;
	setupIDTable(&idt_ptr);
	setupPIC();
	__asm__ __volatile__("lidt %0" : : "m"(idt_ptr)); //tell processor where IDT is
	__asm__ __volatile__("sti"); 
}

void syscall_handler(registers_t *reg) {
	return;
}