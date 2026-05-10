#include <stdint.h>
#include "io.h"
#include "interrupt_handlers.h"
#include "virtual_memory.h"
#include "shell.h"
#include "fat32.h"

uint32_t kernel(){
    cleanScreen();
    setupInterrupts();
    setupDrive();
    shell();
}

