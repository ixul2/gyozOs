#include <stdint.h>
#include "io.h"
#include "lib.h"
#define MEM_SIZE 0x400000
#define FRAME_NUMBER MEM_SIZE/PAGE_SIZE
#define PAGE_TABLE_SIZE 512
#define PAGE_SIZE 4096
#define KERNEL_START 0x40000
#define KERNEL_STACK_TOP 0x80000
#define PROC_START_ADDR 0x100000
#define PROC_SIZE 0X40000

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

#define SEGSEL_KERN_CODE 0x8  // kernel code segment
#define SEGSEL_APP_CODE 0x10  // application code segment
#define SEGSEL_KERN_DATA 0x18 // kernel data segment
#define SEGSEL_APP_DATA 0x20  // application data segment
#define SEGSEL_TASKSTATE 0x28 // task state segment
#define EFLAGS_IF 0x00000200

#define IOPHYSMEM 0x000A0000
#define EXTPHYSMEM 0x00100000
#define PO_FREE 0
#define PO_RESERVED (-1)
#define PO_KERNEL (-2)

// Pseudo-descriptors used for LGDT, LLDT, and LIDT instructions
typedef struct __attribute__((packed, aligned(2))) pseudodescriptor_t{
  uint16_t pseudod_limit; // Limit
  uint64_t pseudod_base;  // Base address
} pseudodescriptor_t;

typedef struct frame_info {
  int8_t owner;
  int8_t refcount;
} frame_info;

typedef uint64_t page_t;

typedef struct{
    page_t pages[PAGE_TABLE_SIZE];
} page_table_t;

typedef page_table_t pml4_t;
typedef page_table_t pdpt_t;
typedef page_table_t pd_t;
typedef page_table_t pt_t;

extern page_table_t *kernel_pagetable;
static frame_info frames_info[FRAME_NUMBER];

uintptr_t alloc_frame(int owner);

void init_virtual_memory(void);
void map_page(pml4_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags, page_table_t* (*allocator)(void));
void change_pagetable(page_table_t* pt);
void pagefault_handler(uintptr_t addr);
