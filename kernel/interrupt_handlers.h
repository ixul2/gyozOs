#ifndef _INTERRUPT_HANDLER_H_
#define _INTERRUPT_HANDLER_H_

#include <stdint.h>
#include "x86-64.h"
#include "io.h"
#include "process.h"
#include "shell.h"
#include "fat32.h"

#define SYS_TIMER_INT 32
#define SYS_KB_INT 33
#define SYS_GETCHAR_INT 0x80
#define SYS_WRITE_CHAR_INT 0x81
#define SYS_CURSOR_INT 0x82
#define SYS_LIST_FILES_INT 0x83
#define SYS_MKDIR_INT 0x84
#define SYS_CD_INT 0x85
#define SYS_RM_INT 0x86
#define SYS_ENABLE_KB_TIM_INT 0x87
#define SYS_YIELD_INT 0x88

// Timer-related constants
#define IO_TIMER1 0x040            /* 8253 Timer #1 */
#define TIMER_MODE (IO_TIMER1 + 3) /* timer mode port */
#define TIMER_SEL0 0x00            /* select counter 0 */
#define TIMER_RATEGEN 0x04         /* mode 2, rate generator */
#define TIMER_16BIT 0x30           /* r/w counter 16 bits, LSB first */

// Timer frequency: (TIMER_FREQ/freq) generates a frequency of 'freq' Hz.
#define TIMER_FREQ 1193182
#define TIMER_DIV(x) ((TIMER_FREQ + (x) / 2) / (x))

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