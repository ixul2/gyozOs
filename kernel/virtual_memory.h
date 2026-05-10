#include <stdint.h>
#define FRAME_NUMBER 1024
#define FRAME_BITMAP_SIZE FRAME_NUMBER/8
#define BITMAP_LOCATION 0x00100000
#define USED_FRAME 0xFFFFFFFF
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE 4096

typedef struct {
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;
} page_t;

typedef struct{
    page_t* pages;
} page_table_t;

typedef struct {
    page_table_t* tables[PAGE_DIRECTORY_SIZE];
    uint32_t table_phys[PAGE_DIRECTORY_SIZE];
    uint32_t phys_addr;
} page_directory_t;

