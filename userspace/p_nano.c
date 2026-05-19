void process_main(){
    int i = 0;
    while(1){
        asm volatile ("int $0x81"
        : 
        : "D"('0' + i), "S"(80*25 - 1)
        : "cc", "memory");
        i = (i+1)%10;
    }
}