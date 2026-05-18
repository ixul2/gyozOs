static inline void sys_write_char(int,char);
void sys_write(char*);
static inline char sys_getchar(void);
static inline void sys_ls(void);
void process_cmd(void);

#define BUFF_LEN 78

char cmd[BUFF_LEN];
int cmd_len = 0;
int cursor = 0;

void process_main(){
    while (1) {
        sys_write("> ");
        while(1){
            char c = sys_getchar();
            if(c == 0x0E){
                if(cmd_len){
                    cmd_len--;
                    sys_write_char(--cursor, 0);
                }
            } else if(c == '\t'){
                for(int i = 0; i<4; i++){
                    if(cmd_len < BUFF_LEN){
                        cmd[cmd_len++] = ' ';
                        sys_write_char(cursor++,' ');
                    }
                }
            } else if(c == '\n') {
                cursor = cursor + 80 - (cursor % 80);
                process_cmd();
                break;
            } else if(cmd_len < BUFF_LEN){
                cmd[cmd_len++] = c;
                sys_write_char(cursor++,c);
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
    if(cmd1_len == 0){
        
    } else if(cmd2_len == 0 && cmd1_len == 2 && cmd1[0] == 'l' && cmd1[1] == 's'){
        sys_ls();
    } else if(cmd2_len == 0 && cmd1_len == 5 && cmd1[0] == 'c' && cmd1[1] == 'l' && cmd1[2] == 'e' && cmd1[3] == 'a' && cmd1[4] == 'n'){
        clear_screen();
        cursor = 0;
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

void sys_write_char(int pos, char c) {
    asm volatile ("int $0x81"
        : 
        : "D"(c), "S"(pos)
        : "cc", "memory");
}

void clear_line(int l){
    for(int i = l*80; i < (l+1)*80; i++){
        sys_write_char(i,0);
    }
}
void clear_screen(void){
    for(int i = 0; i<25; i++){
        clear_line(i);
    }
}

void sys_write(char *s) {
    int i = 0;
    while(s[i] != '\0'){
        char c = s[i];
        if(c == '\n'){
            cursor = cursor + 80 - (cursor % 80);
        } else if(c == '\t'){
            for(int j = 0; j<4; j++){
                sys_write_char(cursor++, ' ');
            }
        } else {
            sys_write_char(cursor++,c);
        }
        i++;
    }
}

void sys_ls(void){
    asm volatile ("int $0x83"
        : 
        :
        : "cc", "memory");
}