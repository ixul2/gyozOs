#include "process.h"

extern page_table_t* kernel_pagetable;
extern uint8_t _binary_userspace__obj_p_shell_bin_start[];
extern uint8_t _binary_userspace__obj_p_shell_bin_end[];

static proc procs[PROC_NUMBER];
proc* current;

typedef struct ramimage {
    void *begin;
    void *end;
} ramimage;

ramimage ramimages[] = {
    {   .begin = _binary_userspace__obj_p_shell_bin_start,
        .end = _binary_userspace__obj_p_shell_bin_end}
};

void init_process(proc* p){
    memset(&p->reg, 0, sizeof(p->reg));
    p->reg.reg_cs = SEGSEL_APP_CODE | 3;
    p->reg.reg_fs = SEGSEL_APP_DATA | 3;
    p->reg.reg_gs = SEGSEL_APP_DATA | 3;
    p->reg.reg_ss = SEGSEL_APP_DATA | 3;
    p->reg.reg_rflags = EFLAGS_IF;

    /*if (flags & PROCINIT_ALLOW_PROGRAMMED_IO) {
        p->p_registers.reg_rflags |= EFLAGS_IOPL_3;
    }
    if (flags & PROCINIT_DISABLE_INTERRUPTS) {
        p->p_registers.reg_rflags &= ~EFLAGS_IF;
    }*/
    p->page_table = kernel_pagetable;
    frames_info[(uintptr_t)kernel_pagetable/PAGE_SIZE].refcount++;
}

void load_process(proc* p, int program) {
    // program argument ignored – always load the embedded binary
    (void)program;

    size_t bin_size = (size_t)(_binary_userspace__obj_p_shell_bin_end -
                               _binary_userspace__obj_p_shell_bin_start);
    size_t pages_needed = (bin_size + PAGE_SIZE - 1) / PAGE_SIZE;

    // 3. User virtual base for this process
    uintptr_t user_base = PROC_START_ADDR + p->id * PROC_SIZE;

    // 4. Allocate and map pages for the binary
    for (size_t i = 0; i < pages_needed; i++) {
        map_page(p->page_table,
                 user_base + i * PAGE_SIZE,
                 user_base + i * PAGE_SIZE,
                 PTE_P | PTE_W | PTE_U,
                 NULL);
    }

    // 5. Copy the embedded binary into user space
    memcpy((void*)user_base, _binary_userspace__obj_p_shell_bin_start, bin_size);

    // 6. Zero out the remainder of the last page (clears any uninitialised data)
    size_t last_page_off = bin_size & (PAGE_SIZE - 1);
    if (last_page_off != 0)
        memset((void*)(user_base + bin_size), 0, PAGE_SIZE - last_page_off);

    // 7. Map a user stack at the top of the region
    map_page(p->page_table,
             user_base + PROC_SIZE - PAGE_SIZE,
             user_base + PROC_SIZE - PAGE_SIZE,
             PTE_P | PTE_W | PTE_U,
             NULL);
    memset((void*)(user_base + PROC_SIZE - PAGE_SIZE), 0, PAGE_SIZE);

    // 8. Set up initial CPU context
    p->reg.reg_rip = user_base;                      // entry point (0x100000 + offset)
    p->reg.reg_rsp = user_base + PROC_SIZE - 8;      // stack grows down

    p->state = P_RUNNABLE;
}

void init_processes(){
    memset((void*) procs, 0, sizeof(procs));
    for(int i = 0; i<PROC_NUMBER; i++){
        procs[i].id = i;
    }
}

void launch_shell(){
    init_processes();
    init_process(&procs[0]);
    load_process(&procs[0],0);
    run(&procs[0]);
}

void run(proc *p) {
  current = p;
  // Load the process's current pagetable.
  change_pagetable(p->page_table);

  exception_return(&p->reg);

spinloop:
  goto spinloop; // should never get here
}