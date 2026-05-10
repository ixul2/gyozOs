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
        fail("No free frame!");
    }
    set_frame(frame);
    return (uintptr_t) frame * PAGE_SIZE;
}

void free_frame(uintptr_t addr){
    clear_frame(addr/PAGE_SIZE);
}

void init_framing(){
    frame_bitmap = (uint32_t*)BITMAP_LOCATION;
    memset(frame_bitmap, 0, FRAME_BITMAP_SIZE * sizeof(uint32_t));
    for(uintptr_t i = 0; i < KERNEL_END; i+=PAGE_SIZE){
	    set_frame(i/PAGE_SIZE);
    }
}

//PAGING

static inline int pml4_index(uintptr_t addr) { return (addr >> 39) & 0x1FF; }
static inline int pdpt_index(uintptr_t addr) { return (addr >> 30) & 0x1FF; }
static inline int pd_index(uintptr_t addr) { return (addr >> 21) & 0x1FF; }
static inline int pt_index(uintptr_t addr) { return (addr >> 12) & 0x1FF; }

static page_t* get_page(pml4_t *pml4, uintptr_t addr, int create)
{
    pdpt_t *pdpt;
    pd_t *pd;
    pt_t *pt;

    page_t pml4e = pml4->pages[pml4_index(addr)];

    if(!(pml4e & PTE_P))
    {
        if(!create) return NULL;

        pdpt = (pdpt_t*)alloc_frame();
        pml4->pages[pml4_index(addr)] =
            ((uintptr_t)pdpt & ~0xFFFUL)
            | PTE_P | PTE_W | PTE_U;
        memset(pdpt, 0, PAGE_SIZE);
    }
    else
    {
        pdpt = (pdpt_t*)(pml4e & ~0xFFFUL);
    }

    page_t pdpte = pdpt->pages[pdpt_index(addr)];

    if(!(pdpte & PTE_P))
    {
        if(!create) return NULL;

        pd = (pd_t*)alloc_frame();
        pdpt->pages[pdpt_index(addr)] =
            ((uintptr_t)pd & ~0xFFFUL)
            | PTE_P | PTE_W;
        memset(pd, 0, PAGE_SIZE);
    }
    else
    {
        pd = (pd_t*)(pdpte & ~0xFFFUL);
    }

    page_t pde = pd->pages[pd_index(addr)];

    if(!(pde & PTE_P))
    {
        if(!create) return NULL;
        pt = (pt_t*)alloc_frame();
        pd->pages[pd_index(addr)] =
            ((uintptr_t)pt & ~0xFFFUL)
            | PTE_P | PTE_W;
        memset(pt, 0, PAGE_SIZE);
    }
    else
    {
        pt = (pt_t*)(pde & ~0xFFFUL);
    }
    return &pt->pages[pt_index(addr)];
}

void map_page(pml4_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags){
    page_t* page = get_page(pml4, virt, 1);
    if(page == NULL) fail("C'est NULL");
    *page = (phys & ~0xFFFUL) | flags | PTE_P;
}

static page_table_t kernel_pagetables[5]
    __attribute__((aligned(4096)));
page_table_t *kernel_pagetable;

void init_virtual_memory()
{
    init_framing();
    kernel_pagetable = &kernel_pagetables[0];
    memset(kernel_pagetables, 0, sizeof(kernel_pagetables));
    kernel_pagetables[0].pages[0] =
        (page_t)&kernel_pagetables[1] | PTE_P | PTE_W | PTE_U;
    kernel_pagetables[1].pages[0] =
        (page_t)&kernel_pagetables[2] | PTE_P | PTE_W | PTE_U;
    kernel_pagetables[2].pages[0] =
        (page_t)&kernel_pagetables[3] | PTE_P | PTE_W | PTE_U;
    kernel_pagetables[2].pages[1] =
        (page_t)&kernel_pagetables[4] | PTE_P | PTE_W | PTE_U;
    for(uintptr_t i = 0; i < MEM_SIZE; i += PAGE_SIZE)
        map_page(kernel_pagetable, i, i, PTE_P | PTE_W |  PTE_U);
    lcr3((uintptr_t) kernel_pagetable);
}

void pagefault_handler(uintptr_t addr){
	
}

