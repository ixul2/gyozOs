#include <stdint.h>

void sys_get_input(const char*);
static inline void sys_write(char c) {
    asm volatile ("int $0x80" : : "D"(c));
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