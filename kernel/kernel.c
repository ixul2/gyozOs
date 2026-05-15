#include <stdint.h>
#include "io.h"
#include "lib.h"
#include "interrupt_handlers.h"
#include "shell.h"
#include "fat32.h"
#include "virtual_memory.h"
#include "process.h"

uint32_t kernel(){
    serial_init();
    cleanScreen();
    init_virtual_memory();
    setupInterrupts();
    setupDrive();
    serial_write("===========================TEST============================\n");
    launch_shell();
    while(1);
}

