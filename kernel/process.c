#include "process.h"

static proc procs[PROC_NUMBER];

void init_process(proc* p, int flags){
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