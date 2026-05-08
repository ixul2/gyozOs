#include <stdint.h>
#include "interrupt_handlers.h"
extern void dummy_handler(void);
extern void keyboard_handler_wrapper(void);

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
	for (int i = 0; i < 256; i++){
		setupIDTEntry(&IDTable[i], (uint64_t) dummy_handler, 0);
	}
	setupIDTEntry(&IDTable[33], (uint64_t) keyboard_handler_wrapper, 0);
	idt_ptr->base = (uint64_t) IDTable;
	idt_ptr->limit = sizeof(IDTable)-1;
}

void setupPIC(){
	__asm__ __volatile__("movb $0x11, %al\n" "outb %al, $0x20"); //send ICW1 to slave and master (initialization)
	__asm__ __volatile__("movb $0x11, %al\n" "outb %al, $0xA0");
	
	__asm__ __volatile__("movb $0x20, %al\n" "outb %al, $0x21");
	__asm__ __volatile__("movb $0x11, %al\n" "outb %al, $0xA1");
	__asm__ __volatile__("movb $0x28, %al\n" "outb %al, $0x20");
	__asm__ __volatile__("movb $0x04, %al\n" "outb %al, $0x21");
	__asm__ __volatile__("movb $0x02, %al\n" "outb %al, $0xA1");
	__asm__ __volatile__("movb $0x01, %al\n" "outb %al, $0x21");
	__asm__ __volatile__("movb $0x01, %al\n" "outb %al, $0xA1");
	__asm__ __volatile__("movb $0xFD, %al\n" "outb %al, $0x21");
	__asm__ __volatile__("movb $0xFF, %al\n" "outb %al, $0xA1");
}

void setupInterrupts(){
	IDT_ptr idt_ptr;
	setupIDTable(&idt_ptr);
	setupPIC();
	__asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
	__asm__ __volatile__("sti");
}


