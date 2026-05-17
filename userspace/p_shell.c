static inline void sys_write_char(char);
static inline void sys_write(char*);
static inline char sys_getchar(void);
static inline void sys_ls(void);
void process_cmd(void);

#define BUFF_LEN 78

char cmd[BUFF_LEN];
int cmd_len = 0;

void process_main(){
    while (1) {
        sys_write_char('>');
        sys_write_char(' ');
        while(1){
            char c = sys_getchar();
            if(c == 0x0E){
                if(cmd_len){
                    cmd_len--;
                    sys_write_char(c);
                }
            } else if(c == '\t'){
                for(int i = 0; i<4; i++){
                    if(cmd_len < BUFF_LEN){
                        cmd[cmd_len++] = ' ';
                        sys_write_char(' ');
                    }
                }
            } else if(c == '\n') {
                sys_write_char(c);
                process_cmd();
                break;
            } else if(cmd_len < BUFF_LEN){
                
                cmd[cmd_len++] = c;
                sys_write_char(c);
            }
        }
    }
}

char cmd1[BUFF_LEN];
int cmd1_len;
char cmd2[BUFF_LEN];
int cmd2_len;
char cmd3[BUFF_LEN];
int cmd3_len;
int too_many;

void parse_cmd(void){
    cmd1_len = 0;
    cmd2_len = 0;
    cmd3_len = 0;
    too_many = 0;
    int ind = 0;
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd1[ind] = cmd[ind];
        cmd1_len++;
        ind++;
    }
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd2[ind] = cmd[ind];
        cmd2_len++;
        ind++;
    }
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd3[ind] = cmd[ind];
        cmd3_len++;
        ind++;
    }
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    if(ind != cmd_len){
        too_many = 1;
    }

}

void process_cmd(void){
    parse_cmd();
    if(cmd2_len == 0 && cmd1_len == 2 && cmd1[0] == 'l' && cmd1[1] == 's'){
        sys_ls();
    } else {
        sys_write("Unknown command\n");
    }
    cmd_len = 0;
}

char sys_getchar() {
    char c;
    asm volatile ("int $0x80"
        : "=a"(c)
        :
        : "cc", "memory");
    return c;
}

void sys_write_char(char c) {
    asm volatile ("int $0x81"
        : 
        : "D"(c)
        : "cc", "memory");
}

void sys_write(char *c) {
    asm volatile ("int $0x82"
        : 
        : "D"(c)
        : "cc", "memory");
}

void sys_ls(void){
    asm volatile ("int $0x83"
        : 
        :
        : "cc", "memory");
}