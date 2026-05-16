#ifndef _INTERRUPT_HANDLER_H_
#define _INTERRUPT_HANDLER_H_

#include <stdint.h>
#include "x86-64.h"
#include "io.h"
#include "process.h"

void setupInterrupts(void);
typedef struct IDT_entry IDT_entry;
struct __attribute__((packed)) IDT_entry {
    uint16_t offset_low;    // bits 0-15 of handler address
    uint16_t selector;      // GDT code segment selector
    uint8_t  ist : 3;       // Interrupt Stack Table index (0-7)
    uint8_t  zero1 : 5;     // Reserved, must be 0
    uint8_t  type_attr;     // Type and attributes
    uint16_t offset_mid;    // bits 16-31 of handler address
    uint32_t offset_high;   // bits 32-63 of handler address
    uint32_t zero2;         // Reserved, must be 0
};

typedef struct IDT_ptr IDT_ptr;
struct IDT_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#endif