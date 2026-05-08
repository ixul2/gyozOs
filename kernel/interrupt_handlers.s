.globl dummy_handler, keyboard_handler_wrapper, print_int_asm;
dummy_handler:
    iretq
    
keyboard_handler_wrapper:
    pushq %rax
    pushq %rcx
    pushq %rdx
    
    movq $0, %rax
    inb $0x60, %al          # read scan code to clear PIC
    
    pushq %rdi 		    # calls keyboard_handler(keycode)
    movq %rax, %rdi
    call keyboard_handler
    pop %rdi

    mov $0x20, %dx          # PIC master command port
    movb $0x20, %al         # EOI command
    outb %al, %dx

    popq %rdx
    popq %rcx
    popq %rax
    iretq

print_int_asm:
    call print_int_int
    iretq
