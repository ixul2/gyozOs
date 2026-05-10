#include <stdint.h>
#include "io.h"
#define MEM_SIZE 0x600000
#define FRAME_NUMBER MEM_SIZE/PAGE_SIZE
#define FRAME_BITMAP_SIZE FRAME_NUMBER/32
#define BITMAP_LOCATION 0x100000
#define USED_FRAME 0xFFFFFFFF
#define PAGE_TABLE_SIZE 512
#define PAGE_SIZE 4096
#define KERNEL_START 0x600000
#define KERNEL_END 0x100000

// The physical address contained in a page table entry
#define PTE_ADDR(pageentry) ((uintptr_t)(pageentry) & ~0xFFFUL)

// Page table entry flags
#define PTE_FLAGS(pageentry) ((page_t)(pageentry) & 0xFFFU)
// - Permission flags: define whether page is accessible
#define PTE_P ((page_t)1) // entry is Present
#define PTE_W ((page_t)2) // entry is Writeable
#define PTE_U ((page_t)4) // entry is User-accessible
// - Accessed flags: automatically turned on by processor
#define PTE_A ((page_t)32)   // entry was Accessed (read/written)
#define PTE_D ((page_t)64)   // entry was Dirtied (written)
#define PTE_PS ((page_t)128) // entry has a large Page Size

typedef uint64_t page_t;

typedef struct{
    page_t pages[PAGE_TABLE_SIZE];
} page_table_t;

typedef page_table_t pml4_t;
typedef page_table_t pdpt_t;
typedef page_table_t pd_t;
typedef page_table_t pt_t;

static inline void lcr3(uintptr_t val) {
  asm volatile("" : : : "memory");
  asm volatile("movq %0,%%cr3" : : "r"(val) : "memory");
}

void init_virtual_memory(void);
