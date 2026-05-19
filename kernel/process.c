#include "process.h"

extern page_table_t* kernel_pagetable;
extern uint8_t _binary_userspace__obj_p_shell_bin_start[];
extern uint8_t _binary_userspace__obj_p_shell_bin_end[];
extern uint8_t _binary_userspace__obj_p_nano_bin_start[];
extern uint8_t _binary_userspace__obj_p_nano_bin_end[];

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

    p->page_table = kernel_pagetable;
    frames_info[(uintptr_t)kernel_pagetable/PAGE_SIZE].refcount++;
}

void load_process(proc* p, int program) {
    // program argument ignored – always load the embedded binary
    (void)program;

    size_t bin_size = (size_t)(_binary_userspace__obj_p_shell_bin_end -
                               _binary_userspace__obj_p_shell_bin_start);
    size_t pages_needed = (bin_size + PAGE_SIZE - 1) / PAGE_SIZE;

    uintptr_t user_base = PROC_START_ADDR + p->id * PROC_SIZE;

    for (size_t i = 0; i < pages_needed; i++) {
        map_page(p->page_table,
                 user_base + i * PAGE_SIZE,
                 user_base + i * PAGE_SIZE,
                 PTE_P | PTE_W | PTE_U,
                 NULL);
    }

    memcpy((void*)user_base, _binary_userspace__obj_p_shell_bin_start, bin_size);

    size_t last_page_off = bin_size & (PAGE_SIZE - 1);
    if (last_page_off != 0)
        memset((void*)(user_base + bin_size), 0, PAGE_SIZE - last_page_off);

    size_t stack_pages = 1;
    for (size_t i = 0; i < stack_pages; i++) {
        map_page(p->page_table,
                user_base + PROC_SIZE - (i + 1) * PAGE_SIZE,
                user_base + PROC_SIZE - (i + 1) * PAGE_SIZE,
                PTE_P | PTE_W | PTE_U,
                NULL);
        memset((void *)(user_base + PROC_SIZE - (i + 1) * PAGE_SIZE), 0, PAGE_SIZE);
    }

    p->reg.reg_rip = user_base;
    p->reg.reg_rsp = user_base + PROC_SIZE - 8;

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

void schedule(){
    int id = (current->id+1)%PROC_NUMBER;
    while(!procs[id].state == P_RUNNABLE){
        id = (id+1)%PROC_NUMBER;
    }
    run(&procs[id]);
}