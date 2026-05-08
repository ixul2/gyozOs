#include <stdint.h>
#include "interrupt_handlers.h"
#include "shell.h"

uint32_t kernel(){
    setupInterrupts();
    shell();
}

