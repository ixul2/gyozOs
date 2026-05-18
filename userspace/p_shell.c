#include <stddef.h>
static inline void sys_write_char(int,char);
void sys_write(char*);
static inline char sys_getchar(void);
static inline void sys_cursor(void);
void sys_ls(void);
void process_cmd(void);
void clear_screen(void);
void set_cursor(int);
void scroll(void);

#define BUFF_LEN 78
#define SCREEN_SIZE 80*25

char cmd[BUFF_LEN];
static char screen[SCREEN_SIZE];
int cmd_len;
int cursor;


void process_main(){
    cmd_len = 0;
    cursor = 0;
    while (1) {
        sys_write("> ");
        while(1){
            char c = sys_getchar();
            if(c == 0x0E){
                if(cmd_len){
                    cmd_len--;
                    sys_write_char(--cursor, 0);
                    screen[cursor] = 0;
                }
            } else if(c == '\t'){
                for(int i = 0; i<4; i++){
                    if(cmd_len < BUFF_LEN){
                        cmd[cmd_len++] = ' ';
                        sys_write_char(cursor++,' ');
                    }
                }
            } else if(c == '\n') {
                set_cursor(cursor + 80 - (cursor % 80));
                process_cmd();
                break;
            } else if(cmd_len < BUFF_LEN){
                cmd[cmd_len++] = c;
                sys_write_char(cursor++,c);
            }
        }
    }
}

char cmd1[BUFF_LEN+1];
int cmd1_len;
char cmd2[BUFF_LEN+1];
int cmd2_len;
char cmd3[BUFF_LEN+1];
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
    cmd1[cmd1_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd2[ind] = cmd[ind];
        cmd2_len++;
        ind++;
    }
    cmd2[cmd2_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd3[ind] = cmd[ind];
        cmd3_len++;
        ind++;
    }
    cmd3[cmd3_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    if(ind != cmd_len){
        too_many = 1;
    }

}

int strcmp(char *a, char *b) {
    int i = 0;
  while (*a && *b && *a == *b && i<= 5) {
    ++a, ++b;
    i++;
  }
  return ((unsigned char)*a > (unsigned char)*b) -
         ((unsigned char)*a < (unsigned char)*b);
}

void process_cmd(void){
    parse_cmd();
    if(cmd1_len == 0){
        
    } else if(cmd2_len == 0 && strcmp(cmd1,"ls") == 0){
        sys_ls();
    } else if(cmd2_len == 0 && strcmp(cmd1,"clean") == 0){
        clear_screen();
        set_cursor(0);
    } else if (cmd2_len == 0 && strcmp(cmd1,"help") == 0){
        sys_write("Availables commands: ls help clean\n");
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
    screen[pos] = c;
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
            set_cursor(cursor + 80 - (cursor % 80));
        } else if(c == '\t'){
            for(int j = 0; j<4; j++){
                sys_write_char(cursor, ' ');
                set_cursor(cursor+1);
            }
        } else {
            sys_write_char(cursor,c);
            set_cursor(cursor+1);
        }
        i++;
    }
}

void scroll(){
    for(int i = 1; i<25;i++){
        int l = (i-1)*80;
        for(int j = 0; j<80; j++){
            screen[l + j - 80] = screen[l + j];
            sys_write_char(l + j - 80,screen[l + j]);
        }
    }
}

void set_cursor(int new_pos) {
    while(new_pos >= SCREEN_SIZE){
        new_pos-=80;
        scroll();
    }
    cursor = new_pos;
}

void sys_cursor() {
    asm volatile("int $0x82"
            :
            : "D"(cursor)
            : "cc", "memory");
}

char file_name[51];

void sys_ls(void){
    int cont = 1;
    while(cont){
        asm  volatile ("int $0x83"
            : "=a"(cont)
            : "D"(file_name)
            : "cc", "memory");
        if(cont){
            sys_write("  ");
            sys_write(file_name);
        }
    }
    sys_write("\n");
}