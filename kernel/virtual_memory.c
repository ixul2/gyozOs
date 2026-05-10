#include "virtual_memory.h"


//FRAME MANAGEMENT

static uint32_t* frame_bitmap;

static inline void set_frame(uint32_t frame){
    frame_bitmap[frame/32] |= (1U << (frame % 32));
}

static inline void clear_frame(uint32_t frame){
    frame_bitmap[frame/32] &= ~(1U << (frame % 32));
}

static inline int is_frame_used(uint32_t frame){
    return frame_bitmap[frame/32] & (1U << (frame % 32));
}

//Finds first free frame from the bitmap

static inline int find_free_frame(){
    for(int i = 0; i<FRAME_BITMAP_SIZE; i++){
        if(frame_bitmap[i] != USED_FRAME){
            for(int j = 0; j<32; j++){
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
    if(frame < 0){
        fail("No free frame\n");
    }
    set_frame(frame);
    return (uintptr_t) frame * PAGE_SIZE;
}

void free_frame(uintptr_t addr){
    clear_frame(addr/PAGE_SIZE);
}

void init_framing(){
    frame_bitmap = (uint32_t*)BITMAP_LOCATION;
    memset(frame_bitmap, 0, FRAME_BITMAP_SIZE);
    for(uintptr_t i = 0; i < KERNEL_END; i+=PAGE_SIZE){
	    set_frame(i/PAGE_SIZE);
    }
}

//PAGING

static inline int pml4_index(uintptr_t addr) { return (addr >> 39) & 0x1FF; }
static inline int pdpt_index(uintptr_t addr) { return (addr >> 30) & 0x1FF; }
static inline int pd_index(uintptr_t addr)   { return (addr >> 21) & 0x1FF; }
static inline int pt_index(uintptr_t addr)   { return (addr >> 12) & 0x1FF; }

static page_t* get_page(pml4_t *pml4, uintptr_t addr, int create)
{
    pdpt_t *pdpt;
    pd_t *pd;
    pt_t *pt;

    page_t pml4e = pml4->pages[pml4_index(addr)];

    if(!(pml4e & PAGE_PRESENT))
    {
        if(!create) return NULL;

        pdpt = (pdpt_t*)alloc_frame();
        memset(pdpt, 0, sizeof(pdpt_t));

        pml4->pages[pml4_index(addr)] =
            ((uintptr_t)pdpt & 0x000FFFFFFFFFF000ULL)
            | PAGE_PRESENT | PAGE_RW;
    }
    else
    {
        pdpt = (pdpt_t*)(pml4e & 0x000FFFFFFFFFF000ULL);
    }

    page_t pdpte = pdpt->pages[pdpt_index(addr)];

    if(!(pdpte & PAGE_PRESENT))
    {
        if(!create) return NULL;

        pd = (pd_t*)alloc_frame();
        memset(pd, 0, sizeof(pd_t));

        pdpt->pages[pdpt_index(addr)] =
            ((uintptr_t)pd & 0x000FFFFFFFFFF000ULL)
            | PAGE_PRESENT | PAGE_RW;
    }
    else
    {
        pd = (pd_t*)(pdpte & 0x000FFFFFFFFFF000ULL);
    }

    page_t pde = pd->pages[pd_index(addr)];

    if(!(pde & PAGE_PRESENT))
    {
        if(!create) return NULL;

        pt = (pt_t*)alloc_frame();
        memset(pt, 0, sizeof(pt_t));

        pd->pages[pd_index(addr)] =
            ((uintptr_t)pt & 0x000FFFFFFFFFF000ULL)
            | PAGE_PRESENT | PAGE_RW;
    }
    else
    {
        pt = (pt_t*)(pde & 0x000FFFFFFFFFF000ULL);
    }

    return &pt->pages[pt_index(addr)];
}

void map_page(pml4_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags){
    page_t* page = get_page(pml4, virt, 1);
    *page = (phys & 0x000FFFFFFFFFF000ULL) | flags | PAGE_PRESENT;
}

static pml4_t *kernel_pml4;

void init_virtual_memory()
{
	init_framing();
    uintptr_t pml4_phys = alloc_frame();
    kernel_pml4 = (pml4_t*)pml4_phys;
    memset(kernel_pml4, 0, sizeof(pml4_t));
    for(uintptr_t i = 0; i < KERNEL_END; i += PAGE_SIZE)
        map_page(kernel_pml4, i, i, PAGE_PRESENT | PAGE_RW);
    while(1);
    asm volatile("mov %0, %%cr3" :: "r"(pml4_phys));
}

void pagefault_handler(uintptr_t addr){
	
}

