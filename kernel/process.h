#include "virtual_memory.h"

#define PROCINIT_ALLOW_PROGRAMMED_IO 0x01
#define PROCINIT_DISABLE_INTERRUPTS 0x02

typedef struct registers {
  uint64_t reg_rax;
  uint64_t reg_rcx;
  uint64_t reg_rdx;
  uint64_t reg_rbx;
  uint64_t reg_rbp;
  uint64_t reg_rsi;
  uint64_t reg_rdi;
  uint64_t reg_r8;
  uint64_t reg_r9;
  uint64_t reg_r10;
  uint64_t reg_r11;
  uint64_t reg_r12;
  uint64_t reg_r13;
  uint64_t reg_r14;
  uint64_t reg_r15;
  uint64_t reg_fs;
  uint64_t reg_gs;

  uint64_t reg_intno; // (3) Interrupt number and error
  uint64_t reg_err;   // code (optional; supplied by x86
                      // interrupt mechanism)

  uint64_t reg_rip;         // (4) Task status: instruction pointer,
  uint16_t reg_cs;          // code segment, flags, stack
  uint16_t reg_padding2[3]; // in the order required by `iret`
  uint64_t reg_rflags;
  uint64_t reg_rsp;
  uint16_t reg_ss;
  uint16_t reg_padding3[3];
} registers;


#define P_FREE 0
#define P_RUNNABLE 1
#define P_BLOCKED (-1)
#define P_BROKEN (-2)

typedef int8_t p_state;

typedef struct proc {
    int8_t id;
    registers reg;
    p_state state;
    page_table_t* page_table;
} proc;

#define PROC_NUMBER 8

void exception_return(registers *reg) __attribute__((noreturn));
void run(proc *p);
void init_process(proc* p);
void init_processes(void);