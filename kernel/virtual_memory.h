#include <stdint.h>
#include "io.h"
#define MEM_SIZE 1024*1024*64
#define FRAME_NUMBER MEM_SIZE/PAGE_SIZE
#define FRAME_BITMAP_SIZE FRAME_NUMBER/32
#define BITMAP_LOCATION 0x100000
#define USED_FRAME 0xFFFFFFFFU
#define PAGE_DIRECTORY_SIZE 512
#define PAGE_TABLE_SIZE 512
#define PAGE_SIZE 4096
#define KERNEL_START 0x40000
#define KERNEL_END 0x100000

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_RW      (1ULL << 1)
#define PAGE_USER    (1ULL << 2)
#define PAGE_NX      (1ULL << 63)

typedef uint64_t page_t;

typedef struct{
    page_t pages[PAGE_TABLE_SIZE];
} page_table_t;

typedef page_table_t pml4_t;
typedef page_table_t pdpt_t;
typedef page_table_t pd_t;
typedef page_table_t pt_t;



void init_virtual_memory(void);
