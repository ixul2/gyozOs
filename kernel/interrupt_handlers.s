.globl keyboard_handler_wrapper, time_handler_wrapper, pagefault_handler_wrapper, sys_getchar_handler_wrapper, print_int_asm, exception_return, sys_write_char_handler_wrapper, sys_list_files_handler_wrapper, sys_cursor_handler_wrapper, sys_mkdir_handler_wrapper, sys_cd_handler_wrapper, sys_rm_handler_wrapper;
    
keyboard_handler_wrapper:
    pushq %rax
    pushq %rcx
    pushq %rdx
    
    movq $0, %rax
    inb $0x60, %al          # read scan code to clear PIC
    
    pushq %rdi             # calls keyboard_handler(keycode)
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

time_handler_wrapper:
    pushq $0
    pushq $32
    jmp generic_exception_handler

print_int_asm:
    call console_print_int_wrapper
    iretq

pagefault_handler_wrapper:
    pushq %rdi
    movq %cr2, %rdi
    call pagefault_handler
    pop %rdi
    iretq

sys_getchar_handler_wrapper:
    pushq $0
    pushq $0x80
    jmp generic_exception_handler

sys_write_char_handler_wrapper:
    pushq $0
    pushq $0x81
    jmp generic_exception_handler

sys_list_files_handler_wrapper:
    pushq $0
    pushq $0x83
    jmp generic_exception_handler

sys_cursor_handler_wrapper:
    pushq $0
    pushq $0x82
    jmp generic_exception_handler

sys_mkdir_handler_wrapper:
    pushq $0
    pushq $0x84
    jmp generic_exception_handler

sys_cd_handler_wrapper:
    pushq $0
    pushq $0x85
    jmp generic_exception_handler

sys_rm_handler_wrapper:
    pushq $0
    pushq $0x86
    jmp generic_exception_handler

generic_exception_handler:
    pushq %gs
    pushq %fs
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rdi
    pushq %rsi
    pushq %rbp
    pushq %rbx
    pushq %rdx
    pushq %rcx
    pushq %rax
    movq %rsp, %rdi
    call exception

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
