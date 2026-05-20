#include "interrupt_handlers.h"

extern proc procs[PROC_NUMBER];
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
extern void sys_kb_tim_handler_wrapper(void);
extern void sys_yield_handler_wrapper(void);
extern void sys_start_handler_wrapper(void);
extern void sys_stop_handler_wrapper(void);
extern void sys_read_handler_wrapper(void);
extern void sys_write_handler_wrapper(void);
extern volatile int key_ready;
extern char last_key;
extern void ls(void);

extern int kb_tail;
extern int kb_head;
extern char kb_buffer[KB_BUFFER_SIZE];
extern proc *keyboard_waiting;

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
    setupIDTEntry(&IDTable[SYS_KB_INT], (uint64_t) keyboard_handler_wrapper, 0); //keyboard
    setupIDTEntry(&IDTable[48], (uint64_t) print_int_asm, 3); //custom interrupts
    setupIDTEntry(&IDTable[SYS_GETCHAR_INT], (uint64_t) sys_getchar_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_WRITE_CHAR_INT], (uint64_t) sys_write_char_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_CURSOR_INT], (uint64_t) sys_cursor_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_LIST_FILES_INT], (uint64_t) sys_list_files_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_MKDIR_INT], (uint64_t) sys_mkdir_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_CD_INT], (uint64_t) sys_cd_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_RM_INT], (uint64_t) sys_rm_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_ENABLE_KB_TIM_INT], (uint64_t) sys_kb_tim_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_YIELD_INT], (uint64_t) sys_yield_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_START_INT], (uint64_t) sys_start_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_STOP_INT], (uint64_t) sys_stop_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_READ_INT], (uint64_t) sys_read_handler_wrapper, 3);
    setupIDTEntry(&IDTable[SYS_WRITE_INT], (uint64_t) sys_write_handler_wrapper, 3);
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
    
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);

    outb(0x21, 0xFF);   // mask all
    outb(0xA1, 0xFF);
}

void setupInterrupts(){
    IDT_ptr idt_ptr;
    setupIDTable(&idt_ptr);
    setupPIC();
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr)); //tell processor where IDT is
    __asm__ __volatile__("sti" :::);    
}

int reschedule = 0;

void sys_write_char_handler(registers_t *reg) {
    shell_print_char((int)reg->reg_rsi, (char)reg->reg_rdi);
}


void sys_getchar_handler(void) {
    proc* p = current;
    if (kb_head == kb_tail) {
        p->state = P_BLOCKED;
        keyboard_waiting = p;
        reschedule = 1;
    } else {
        char val = kb_buffer[kb_tail];
        kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
        p->reg.reg_rax = val;
    }
}

void sys_list_files_handler(registers_t *reg){
    int cont = list_files((char*)reg->reg_rdi);
    current->reg.reg_rax = cont;
}

void sys_cursor_handler(registers_t *reg){
    outb(0x3D4, 0x0F);                // cursor location low byte
    outb(0x3D5, (uint8_t)(reg->reg_rdi & 0xFF));

    outb(0x3D4, 0x0E);                // cursor location high byte
    outb(0x3D5, (uint8_t)((reg->reg_rdi >> 8) & 0xFF));
}

void sys_mkdir_handler(registers_t* reg){
    make_directory((char*)reg->reg_rdi);
}

void sys_cd_handler(registers_t* reg){
    change_directory((int)reg->reg_rdi);
}

void sys_rm_handler(registers_t* reg){
    remove_directory((int)reg->reg_rdi);
}

void time_handler(){
    reschedule = 1;
    outb(0x20, 0x20);
}

void sys_yield(){
    reschedule = 1;
}

void exception(registers_t* reg){
    current->reg = *reg;
    change_pagetable(kernel_pagetable);
    switch(reg->reg_intno){
        case SYS_GETCHAR_INT:
            sys_getchar_handler();
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
            time_handler();
            break;
        
        case SYS_KB_INT:
            keyboard_handler();
            break;
        
        case SYS_ENABLE_KB_TIM_INT:
            outb(0x21, 0xFC);
            outb(0xA1, 0xFF);
            break;

        case SYS_YIELD_INT:
            sys_yield();
            break;

        case SYS_START_INT:
            sys_start(reg);
            break;

        case SYS_STOP_INT:
            sys_stop(reg);
            break;

        case SYS_READ_INT:
            read_file(reg->reg_rdi, reg->reg_rsi);
            break;

        case SYS_WRITE_INT:
            write_file(reg->reg_rdi, reg->reg_rsi, reg->reg_rdx);
            break;
    }
    if(!reschedule && current->state == P_RUNNABLE){
        run(current);
    } else {
        reschedule = 0;
        schedule();
    }
}
