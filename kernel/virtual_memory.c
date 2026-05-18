#include "virtual_memory.h"

// Segments
static uint64_t segments[7];

static void set_app_segment(uint64_t *segment, uint64_t type, int dpl) {
  *segment = type | X86SEG_S                     // code/data segment
             | ((uint64_t)dpl << 45) | X86SEG_P; // segment present
}

static void set_sys_segment(uint64_t *segment, uint64_t type, int dpl,
                            uintptr_t addr, size_t size) {
  segment[0] = ((addr & 0x0000000000FFFFFFUL) << 16) |
               ((addr & 0x00000000FF000000UL) << 32) |
               ((size - 1) & 0x0FFFFUL) | (((size - 1) & 0xF0000UL) << 48) |
               type | ((uint64_t)dpl << 45) | X86SEG_P; // segment present
  segment[1] = addr >> 32;
}

static tss kernel_task_descriptor;

void segments_init(void) {
  // Segments for kernel & user code & data
  // The privilege level, which can be 0 or 3, differentiates between
  // kernel and user code. (Data segments are unused in WeensyOS.)
  segments[0] = 0;
  set_app_segment(&segments[SEGSEL_KERN_CODE >> 3], X86SEG_X | X86SEG_L, 0);
  set_app_segment(&segments[SEGSEL_APP_CODE >> 3], X86SEG_X | X86SEG_L, 3);
  set_app_segment(&segments[SEGSEL_KERN_DATA >> 3], X86SEG_W, 0);
  set_app_segment(&segments[SEGSEL_APP_DATA >> 3], X86SEG_W, 3);
  set_sys_segment(&segments[SEGSEL_TASKSTATE >> 3], X86SEG_TSS, 0,
                  (uintptr_t)&kernel_task_descriptor,
                  sizeof(kernel_task_descriptor));

  x86_64_pseudodescriptor gdt;
  gdt.pseudod_limit = sizeof(segments) - 1;
  gdt.pseudod_base = (uint64_t)segments;

  // Kernel task descriptor lets us receive interrupts
  memset(&kernel_task_descriptor, 0, sizeof(kernel_task_descriptor));
  kernel_task_descriptor.rsp0 = KERNEL_STACK_TOP;
  kernel_task_descriptor.iopb_offset = sizeof(kernel_task_descriptor);

  // Reload segment pointers
  asm volatile("lgdt %0\n\t"
               "ltr %1\n\t"
               :
               : "m"(gdt), "r"((uint16_t)SEGSEL_TASKSTATE)
               : "memory");

  // Set up control registers: check alignment
  uint32_t cr0 = rcr0();
  cr0 |= CR0_PE | CR0_PG | CR0_WP | CR0_AM | CR0_MP | CR0_NE;
  lcr0(cr0);
}

//FRAME MANAGEMENT

int physical_memory_isreserved(uintptr_t pa) {
  return pa == 0 || (pa >= IOPHYSMEM && pa < EXTPHYSMEM);
}

static frame_info frames_info[FRAME_NUMBER]
    __attribute__((aligned(4096)));

//Finds first free frame from the bitmap

static inline int find_free_frame(){
    for(int i = 0; i<FRAME_NUMBER; i++){
        if(frames_info[i].owner == PO_FREE){
            return i;
        }
    }
    return -1;
}

uintptr_t alloc_frame(int8_t owner){
    int frame = find_free_frame();
    assert(frame >= 0, "No free frame available");
    uintptr_t phys = (uintptr_t)(frame * PAGE_SIZE);
    memset((void*) phys, 0, PAGE_SIZE);

    frames_info[frame].owner = owner;
    frames_info[frame].refcount++;
    return phys;
}

int assign_physical_page(uintptr_t addr, int8_t owner) {
  if ((addr & 0xFFF) != 0 || addr >= MEM_SIZE ||
      frames_info[addr/PAGE_SIZE].refcount != 0) {
    return -1;
  } else {
    frames_info[addr/PAGE_SIZE].refcount = 1;
    frames_info[addr/PAGE_SIZE].owner = owner;
    memset((void *)addr, 0, PAGE_SIZE);
    return 0;
  }
}

//Allocate frame and returns its physical address

void init_framing(){
    extern char end[];

    for(int frame = 0; frame<FRAME_NUMBER; frame+=1){
        uintptr_t addr = frame * PAGE_SIZE;
        if(physical_memory_isreserved(addr)){
            frames_info[frame].owner = PO_RESERVED;
        } else if(addr >= KERNEL_START && addr < (uintptr_t)end){
            frames_info[frame].owner = PO_KERNEL;
        } else {
            frames_info[frame].owner = PO_FREE;
        }
        frames_info[frame].refcount = (frames_info[frame].owner != PO_FREE);
    }
}

//PAGING

static inline int pml4_index(uintptr_t addr) { return (addr >> 39) & 0x1FF; }
static inline int pdpt_index(uintptr_t addr) { return (addr >> 30) & 0x1FF; }
static inline int pd_index(uintptr_t addr) { return (addr >> 21) & 0x1FF; }
static inline int pt_index(uintptr_t addr) { return (addr >> 12) & 0x1FF; }

static page_t* get_page(pml4_t *pml4, uintptr_t addr, page_table_t* (*allocator)(void))
{
    pdpt_t *pdpt;
    pd_t *pd;
    pt_t *pt;

    page_t pml4e = pml4->pages[pml4_index(addr)];

    if(!(pml4e & PTE_P))
    {
        if(!allocator) return NULL;

        pdpt = (pdpt_t*) allocator();
        if(!pdpt) return NULL;

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
        if(!allocator) return NULL;

        pd = (pd_t*) allocator();
        if(!pd) return NULL;

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
        if(!allocator) return NULL;

        pt = (pt_t*) allocator();
        if(!pt) return NULL;

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

void map_page(pml4_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags, page_table_t* (*allocator)(void)){
    page_t* page = get_page(pml4, virt, allocator);
    assert(page != NULL, "C'est NULL");
    *page = (phys & ~0xFFFUL) | flags;
}

static page_table_t kernel_pagetables[5]
    __attribute__((aligned(PAGE_SIZE)));
page_table_t *kernel_pagetable;

void change_pagetable(page_table_t *page_table){
    lcr3((uintptr_t) page_table);
}

void init_virtual_memory()
{
    segments_init();
    kernel_pagetable = &kernel_pagetables[0];
    memset(kernel_pagetables, 0, sizeof(kernel_pagetables));
    kernel_pagetables[0].pages[0] =
        (page_t)&kernel_pagetables[1] | PTE_P | PTE_W | PTE_U;
    kernel_pagetables[1].pages[0] =
        (page_t)&kernel_pagetables[2] | PTE_P | PTE_W | PTE_U;
    for(int i = 0; i<2; i++){
        kernel_pagetables[2].pages[i] =
            (page_t)&kernel_pagetables[i+3] | PTE_P | PTE_W | PTE_U;
    }
    for(uintptr_t i = 0x000000; i < MEM_SIZE; i += PAGE_SIZE)
        map_page(kernel_pagetable, i, i, PTE_P | PTE_W , NULL);
    change_pagetable(kernel_pagetable);
    init_framing();
}

void pagefault_handler(uintptr_t addr){
    fail("PAGE FAULT");
}

