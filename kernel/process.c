#include "process.h"

extern page_table_t* kernel_pagetable;
extern uint8_t _binary_userspace__obj_p_shell_bin_start[];
extern uint8_t _binary_userspace__obj_p_shell_bin_end[];
extern uint8_t _binary_userspace__obj_p_dummy1_bin_start[];
extern uint8_t _binary_userspace__obj_p_dummy1_bin_end[];
extern uint8_t _binary_userspace__obj_p_dummy2_bin_start[];
extern uint8_t _binary_userspace__obj_p_dummy2_bin_end[];
extern uint8_t _binary_userspace__obj_p_idle_bin_start[];
extern uint8_t _binary_userspace__obj_p_idle_bin_end[];

static proc procs[PROC_NUMBER];
proc* current;

typedef struct ramimage {
    void *begin;
    void *end;
} ramimage;

//Stores the location of all the programs
ramimage ramimages[] = {
    {   .begin = _binary_userspace__obj_p_idle_bin_start,
        .end = _binary_userspace__obj_p_idle_bin_end },
    {   .begin = _binary_userspace__obj_p_shell_bin_start,
        .end = _binary_userspace__obj_p_shell_bin_end },
    {   .begin = _binary_userspace__obj_p_dummy1_bin_start,
        .end = _binary_userspace__obj_p_dummy1_bin_end },
    {   .begin = _binary_userspace__obj_p_dummy2_bin_start,
        .end = _binary_userspace__obj_p_dummy2_bin_end }
};

void init_process(proc* p){
    memset(&p->reg, 0, sizeof(p->reg));
    p->reg.reg_cs = SEGSEL_APP_CODE | 3;
    p->reg.reg_fs = SEGSEL_APP_DATA | 3;
    p->reg.reg_gs = SEGSEL_APP_DATA | 3;
    p->reg.reg_ss = SEGSEL_APP_DATA | 3;
    p->reg.reg_rflags = EFLAGS_IF;

    //All processes share the same table as the kernel
    p->page_table = kernel_pagetable;
    frames_info[(uintptr_t)kernel_pagetable/PAGE_SIZE].refcount++;
}

void load_process(proc* p, int program) {
    ramimage prog = ramimages[program];

    size_t bin_size = (size_t)((size_t)prog.end - (size_t)prog.begin);
    size_t pages_needed = (bin_size + PAGE_SIZE - 1) / PAGE_SIZE;

    //User_base must equal the address in the linker of the program
    uintptr_t user_base = PROC_START_ADDR + (p->id) * PROC_SIZE;

    //Maps all the required pages to store the program
    for (size_t i = 0; i < pages_needed; i++) {
        map_page(p->page_table,
            user_base + i * PAGE_SIZE,
            user_base + i * PAGE_SIZE,
            PTE_P | PTE_W | PTE_U,
            NULL);
        assign_physical_page(user_base + i * PAGE_SIZE,p->id);
    }

    memcpy((void*)user_base, prog.begin, bin_size);

    size_t last_page_off = bin_size & (PAGE_SIZE - 1);
    if (last_page_off != 0)
        memset((void*)(user_base + bin_size), 0, PAGE_SIZE - last_page_off);

    //Maps the stack
    size_t stack_pages = 1;
    for (size_t i = 0; i < stack_pages; i++) {
        map_page(p->page_table,
            user_base + PROC_SIZE - (i + 1) * PAGE_SIZE,
            user_base + PROC_SIZE - (i + 1) * PAGE_SIZE,
            PTE_P | PTE_W | PTE_U,
            NULL);
        assign_physical_page(user_base + i * PAGE_SIZE,p->id);assign_physical_page(user_base + PROC_SIZE - (i + 1) * PAGE_SIZE,p->id);
        memset((void *)(user_base + PROC_SIZE - (i + 1) * PAGE_SIZE), 0, PAGE_SIZE);
    }

    //Sets entry point and stack
    p->reg.reg_rip = user_base;
    p->reg.reg_rsp = user_base + PROC_SIZE - 8;

    p->state = P_RUNNABLE;
}

//Initializes all the processes
void init_processes(){
    memset((void*) procs, 0, sizeof(procs));
    for(int i = 0; i < PROC_NUMBER; i++){
        procs[i].id = i;
        init_process(&procs[i]);
    }
    //Idle process is always running
    load_process(&procs[0],0);
}

void launch_shell(){
    load_process(&procs[1],1);
    run(&procs[1]);
}

void run(proc *p) {
    current = p;
    // Load the process's current pagetable.
    change_pagetable(p->page_table);
    // Changes the value of the registers to be equal to those stored in p->reg
    exception_return(&p->reg);

spinloop:
    goto spinloop; // should never get here
}

void schedule(){
    int id = current->id;
    for(int i = 0; i<PROC_NUMBER + 1; i++){
        id = (id + 1)%PROC_NUMBER;
        if(id == 0){
            id = 1;
        }
        if(procs[id].state == P_RUNNABLE){
            run(&procs[id]);
        }
    }
    //If no runnable process, goes idle
    run(&procs[0]);
}

//Called to start any of the dummy processes
void sys_start(registers_t* reg){
    assert(reg->reg_rdi == 2 || reg->reg_rdi == 3,"Not a startable process");
    assert(procs[reg->reg_rdi].state == P_FREE,"Process already running");
    load_process(&procs[reg->reg_rdi],reg->reg_rdi);
}

//Called to stop any of the dummy processes
void sys_stop(registers_t* reg){
    assert(reg->reg_rdi == 2 || reg->reg_rdi == 3,"Not a stoppable process");
    assert(procs[reg->reg_rdi].state == P_RUNNABLE,"Process already running");
    procs[reg->reg_rdi].state = P_FREE;
}