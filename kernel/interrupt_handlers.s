.globl keyboard_handler_wrapper, time_handler_wrapper, pagefault_handler_wrapper, sys_getchar_handler_wrapper, print_int_asm, exception_return, sys_write_char_handler_wrapper, sys_list_files_handler_wrapper, sys_cursor_handler_wrapper, sys_mkdir_handler_wrapper, sys_cd_handler_wrapper, sys_rm_handler_wrapper, sys_kb_tim_handler_wrapper, sys_yield_handler_wrapper;
    
time_handler_wrapper:
    pushq $0
    pushq $32
    jmp generic_exception_handler

keyboard_handler_wrapper:
    pushq $0
    pushq $33
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

sys_kb_tim_handler_wrapper:
    pushq $0
    pushq $0x87
    jmp generic_exception_handler

sys_yield_handler_wrapper:
    pushq $0
    pushq $0x88
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
