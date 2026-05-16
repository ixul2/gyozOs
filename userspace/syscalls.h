#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <stdint.h>

void sys_get_input(const char*);
static inline void sys_write(char c) {
    asm volatile ("int $0x80" : : "D"(c));
}
static inline void sys_call(void) {
  asm volatile("int %0"
               : /* no result */
               : "i"(0x80)
               : "cc", "memory");
spinloop:
  goto spinloop; // should never get here
}
void sys_allocpage(uintptr_t);
void sys_ls(void);
void sys_cd(const char*);
void sys_nano(const char*);
void sys_touch(const char*);
void sys_help(void);
void sys_clean(void);
void sys_yield(void);
void sys_fork(void);
#endif