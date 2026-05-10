#include <stdint.h>
#include "interrupt_handlers.h"
#include "shell.h"
#include "fat32.h"

uint32_t kernel(){
    setupInterrupts();
    setupDrive();
    while(1);
    shell();
}

