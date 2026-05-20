void process_main(){
    asm volatile("int $0x87" ::: "cc", "memory");
    int i = 0;
    while(1){
        if(i%2000 == 0){
            int j = i/2000;
            asm volatile ("int $0x81"
            : 
            : "D"('#' * (j/25)), "S"(80*(j%25) + 78)
            : "cc", "memory");
        }
        i = (i+1)%100000;
        asm volatile("int $0x88" ::: "memory");
    }
}