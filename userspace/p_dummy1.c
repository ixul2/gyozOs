void process_main(){
    asm volatile("int $0x87" ::: "cc", "memory");
    int i = 0;
    while(1){
        if(i%1000 == 0){
            int j = i/1000;
            asm volatile ("int $0x81"
            : 
            : "D"('#' * (j/25)), "S"(80*(j%25) + 76)
            : "cc", "memory");
        }
        i = (i+1)%50000;
        asm volatile("int $0x88" ::: "memory");
    }
}