#include "virtual_memory.h"
#include "io.h"

void memory_init(void){
    uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__ ((aligned(PAGE_SIZE)));
    for(uint32_t i = 0; i<PAGE_DIRECTORY_SIZE; i++){
        page_directory[i] = 0x00000002;
    }
    uint32_t first_page_table[PAGE_TABLE_SIZE] __attribute__ ((aligned(PAGE_SIZE)));
    for(uint32_t i = 0; i<PAGE_TABLE_SIZE; i++){
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }
    page_directory[0] =
        ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
}