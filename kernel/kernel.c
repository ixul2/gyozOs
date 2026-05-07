#include <stdint.h>
#include "interrupt_handlers.h"

uint32_t kernel(){
    setupInterrupts();
    while(1);
}

