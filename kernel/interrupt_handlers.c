#include "interrupt_handlers.h"

extern proc* current;
extern void time_handler_wrapper(void);
extern void keyboard_handler_wrapper(void);
extern void pagefault_handler_wrapper(void);
extern void syscall_handler_wrapper(void);
extern void print_int_asm(void);
extern void sys_getchar_handler_wrapper(void);
extern void sys_write_char_handler_wrapper(void);
extern void sys_cursor_handler_wrapper(void);
extern void sys_list_files_handler_wrapper(void);
extern void sys_mkdir_handler_wrapper(void);
extern void sys_cd_handler_wrapper(void);
extern void sys_rm_handler_wrapper(void);
extern volatile int key_ready;
extern char last_key;
extern void ls(void);

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
    setupIDTEntry(&IDTable[SYS_TIMER_INT], (uint64_t) time_handler_wrapper, 0);
    setupIDTEntry(&IDTable[33], (uint64_t) keyboard_handler_wrapper, 0); //keyboard
    setupIDTEntry(&IDTable[48], (uint64_t) print_int_asm, 3); //custom interrupts
    setupIDTEntry(&IDTable[SYS_GETCHAR_INT], (uint64_t) sys_getchar_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_WRITE_CHAR_INT], (uint64_t) sys_write_char_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_CURSOR_INT], (uint64_t) sys_cursor_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_LIST_FILES_INT], (uint64_t) sys_list_files_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_MKDIR_INT], (uint64_t) sys_mkdir_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_CD_INT], (uint64_t) sys_cd_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_RM_INT], (uint64_t) sys_rm_handler_wrapper, 3);
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
    
    outb(0x21, 0xFD); //unmask keyboard IRQ and time IRQ
    outb(0xA1, 0xFF);
}

void setupInterrupts(){
    IDT_ptr idt_ptr;
    setupIDTable(&idt_ptr);
    setupPIC();
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr)); //tell processor where IDT is
    __asm__ __volatile__("sti"); 
}

int reschedule = 0;

void sys_write_char_handler(registers_t *reg) {
    shell_print_char((int)reg->reg_rsi, (char)reg->reg_rdi);
    run(current);
}


void sys_getchar_handler(registers_t *reg) {
    while(!key_ready){
        asm volatile("sti; hlt; cli" ::: "memory");
    }
    key_ready = 0;
    current->reg.reg_rax = last_key;
}

void sys_list_files_handler(registers_t *reg){
    int cont = list_files((char*)reg->reg_rdi);
    current->reg.reg_rax = cont;
    run(current);
}

void sys_cursor_handler(registers_t *reg){
    outb(0x3D4, 0x0F);                // cursor location low byte
    outb(0x3D5, (uint8_t)(reg->reg_rdi & 0xFF));

    outb(0x3D4, 0x0E);                // cursor location high byte
    outb(0x3D5, (uint8_t)((reg->reg_rdi >> 8) & 0xFF));
    run(current);
}

void sys_mkdir_handler(registers_t* reg){
    make_directory((char*)reg->reg_rdi);
    run(current);
}

void sys_cd_handler(registers_t* reg){
    change_directory((int)reg->reg_rdi);
    run(current);
}

void sys_rm_handler(registers_t* reg){
    remove_directory((int)reg->reg_rdi);
    run(current);
}

void time_handler(registers_t* reg){
    current->reg = *reg;
    schedule();
}

void exception(registers_t* reg){
    current->reg = *reg;
    change_pagetable(kernel_pagetable);
    switch(reg->reg_intno){
        case SYS_GETCHAR_INT:
            sys_getchar_handler(reg);
            break;

        case SYS_WRITE_CHAR_INT:
            sys_write_char_handler(reg);
            break;

        case SYS_LIST_FILES_INT:
            sys_list_files_handler(reg);
            break;
        
        case SYS_CURSOR_INT:
            sys_cursor_handler(reg);
            break;
        
        case SYS_MKDIR_INT:
            sys_mkdir_handler(reg);
            break;

        case SYS_CD_INT:
            sys_cd_handler(reg);
            break;

        case SYS_RM_INT:
            sys_rm_handler(reg);
            break;

        case SYS_TIMER_INT:
            time_handler(reg);
            break;
    }
    if(!reschedule && current->state == P_RUNNABLE){
        run(current);
    } else {
        schedule();
    }
}
