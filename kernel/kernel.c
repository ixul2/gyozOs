#include <stdint.h>
#include "interrupt_handlers.h"
#include "virtual_memory.h"
#include "shell.h"
#include "fat32.h"

uint32_t kernel(){
    setupInterrupts();
    setupDrive();
    init_virtual_memory();
    while(1);
    shell();
}

