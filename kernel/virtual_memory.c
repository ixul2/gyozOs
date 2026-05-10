#include "virtual_memory.h"
#include "io.h"

extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern void load_page_directory(uint32_t*);

//FRAME MANAGEMENT

static uint32_t* frame_bitmap;

static inline void set_frame(uint32_t frame){
    frame_bitmap[frame/32] |= (1 << (frame % 32));
}

static inline void clear_frame(uint32_t frame){
    frame_bitmap[frame/32] &= ~(1 << (frame % 32));
}

static inline int is_frame_used(uint32_t frame){
    frame_bitmap[frame/32] & (1 << (frame % 32));
}

//Finds first free frame from the bitmap

static inline int find_free_frame(){
    for(int i = 0; i<FRAME_BITMAP_SIZE; i++){
        if(frame_bitmap[i] != USED_FRAME){
            for(int j = 0; j<8; j++){
                if (!is_frame_used(32*i + j)){
                    return 32*i + j;
                }
            }
        }
    }
    return -1;
}

//Allocate frame and returns its physical address

uintptr_t alloc_frame(){
    int frame = find_free_frame();
    assert(frame != -1);
    set_frame(frame);
    return frame * PAGE_SIZE;
}

void free_frame(uint32_t addr){
    clear_frame(addr/PAGE_SIZE);
}

void init_frame_manager(){
    frame_bitmap = (uint32_t*)BITMAP_LOCATION;
    for(int i = 0; i < FRAME_BITMAP_SIZE; i++){
        frame_bitmap[i] = 0x0;
    }
    for(int i = kernel_start; i < kernel_end; i+=PAGE_SIZE){
        set_frame(i/PAGE_SIZE);
    }
    set_frame(0);
}

//PAGING

page_directory_t* current_directory;
page_directory_t* kernel_directory;

page_t* get_page(uint32_t addr, int create, page_directory_t* dir){
    addr /= PAGE_SIZE;
    
    uint32_t tab_index = addr/PAGE_DIRECTORY_SIZE;
    if(dir->tables[tab_index]){
        return &dir->tables[tab_index]->pages[addr % PAGE_TABLE_SIZE];
    } else if(create){
        page_table_t* table = (page_table_t*) alloc_frame();
        memset(table, 0, sizeof(page_table_t));
        dir->tables[tab_index] = table;
        dir->table_phys[tab_index] = ((uintptr_t)table) | 0x7;
        return &(table->pages[addr % PAGE_TABLE_SIZE]);
    }
    return NULL;
}

void map_page(uint32_t virt_addr, uint32_t phys_addr, page_directory_t* dir){
    page_t* page = get_page(virt_addr,1,dir);

    page->present = 1;
    page->rw = 1;
    page->user = 0;
    page->frame = phys_addr/PAGE_SIZE;
    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

void change_directory(page_directory_t* dir){
    current_directory = dir;
    load_page_directory(dir->table_phys);
}



void init_virtual_memory(){
    init_frame_manager();
}
