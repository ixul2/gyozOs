#include "process.h"

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

void load_process(proc* p, int program){
    uintptr_t va = PROC_START_ADDR + p->id * PROC_SIZE;
    uintptr_t stack = PROC_START_ADDR + (p->id + 1) * PROC_SIZE - PAGE_SIZE;

    for(uintptr_t addr = va; addr <= stack; addr += PAGE_SIZE){
        assign_physical_page(addr, p->id);
        map_page(p->page_table, addr, addr, PTE_P | PTE_W | PTE_U, NULL);
    }
    p->reg.reg_rip = 0x100000;
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

  // This function is defined in k-exception.S. It restores the process's
  // registers then jumps back to user mode.
  exception_return(&p->reg);

spinloop:
  goto spinloop; // should never get here
}