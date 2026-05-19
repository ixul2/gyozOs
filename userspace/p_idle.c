void process_main(){
    asm volatile("sti; hlt" ::: "memory");
}