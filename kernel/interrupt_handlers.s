.globl dummy_handler, keyboard_handler_wrapper, pagefault_handler_wrapper, syscall_handler_wrapper, print_int_asm, exception_return;
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
    call console_print_int_wrapper
    iretq


pagefault_handler_wrapper:
    pushq %rdi
    movq %cr2, %rdi
    call pagefault_handler
    pop %rdi
    iretq

syscall_handler_wrapper:
    iretq

exception_return:
    movq %rdi, %rsp
    popq %rax
    popq %rcx
    popq %rdx
    popq %rbx
    popq %rbp
    popq %rsi
    popq %rdi
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %fs
    popq %gs
    addq $16, %rsp
    iretq

.section .data
pos:
    .long 0