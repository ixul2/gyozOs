#include <stddef.h>
static inline void sys_write_char(int,char);
void sys_write(char*);
static inline char sys_getchar(void);
static inline void sys_cursor(void);
static inline void sys_mkdir(void);
static inline void sys_cd(int);
static inline void sys_rm(int);

void sys_ls(void);
void process_cmd(void);
void clear_screen(void);
void set_cursor(int);
void scroll(void);

#define BUFF_LEN 78
#define SCREEN_SIZE 80*25

char cmd[BUFF_LEN];
int cmd_len;
static char screen[SCREEN_SIZE];
char path[51];
int path_len;
int cmd_ind;
int cursor;
int backward_steps_to_make = 0;

void process_main(){
    cmd_len = 0;
    cursor = 0;
    path[0] = '\0';
    while (1) {
        sys_write(path);
        sys_write("> ");
        while(1){
            sys_cursor();
            char c = sys_getchar();
            if(c == 0x0E){
                if(cmd_ind){
                    if(cmd_ind == cmd_len){
                        cmd_len--;
                    }
                    cmd_ind--;
                    sys_write_char(--cursor, 0);
                    cmd[cmd_ind] = ' ';
                    screen[cursor] = ' ';
                }
            } else if(c == 1){
                if(cmd_ind != 0){
                    cmd_ind--;
                    set_cursor(cursor - 1);
                }
            } else if(c == 2){
                if(cmd_len != cmd_ind){
                    cmd_ind++;
                    set_cursor(cursor + 1);
                }
            } else if(c == 3){
                if(cursor - 80 >= 0){
                    scroll();
                    set_cursor(cursor - 80);
                }
            } else if(c == '\t'){
                for(int i = 0; i<4; i++){
                    if(cmd_ind < cmd_len){
                        cmd[cmd_ind++] = ' ';
                        sys_write_char(cursor++,' ');
                    } else if(cmd_len < BUFF_LEN){
                        cmd_ind++;
                        cmd[cmd_len++] = ' ';
                        sys_write_char(cursor++,' ');
                    }
                }
            } else if(c == '\n') {
                set_cursor(cursor + 80 - (cursor % 80));
                process_cmd();
                break;
            } else if(cmd_len < BUFF_LEN){
                if(cmd_ind < cmd_len){
                    cmd[cmd_ind++] = c;
                    sys_write_char(cursor++,c);
                } else if(cmd_len < BUFF_LEN){
                    cmd_ind++;
                    cmd[cmd_len++] = c;
                    sys_write_char(cursor++,c);
                }
            }
        }
    } 
}

char cmd1[BUFF_LEN+1];
int cmd1_len;
char cmd2[BUFF_LEN+4];
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
        cmd1[cmd1_len] = cmd[ind];
        cmd1_len++;
        ind++;
    }
    cmd1[cmd1_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd2[cmd2_len] = cmd[ind];
        cmd2_len++;
        ind++;
    }
    cmd2[cmd2_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    while(ind < cmd_len && cmd[ind] != ' '){
        cmd3[cmd3_len] = cmd[ind];
        cmd3_len++;
        ind++;
    }
    cmd3[cmd3_len] = '\0';
    while(ind < cmd_len && cmd[ind] == ' '){
        ind++;
    }
    if(cmd[ind] != '\0'){
        too_many = 1;
    }
}

char file_name_return[51];

int strcmp(char *a, char *b) {
    int i = 0;
  while (*a && *b && *a == *b && i<= 5) {
    ++a, ++b;
    i++;
  }
  return ((unsigned char)*a > (unsigned char)*b) -
         ((unsigned char)*a < (unsigned char)*b);
}

int ind_of_file_in_current_directory(char* active_buffer){
    int ind = 0;
    int found = -1;
    while(ind != -1){
        asm volatile ("int $0x83"
            : "=a"(ind)
            : "D"(file_name_return)
            : "cc", "memory");
        if(ind != -1 && strcmp(file_name_return,active_buffer) == 0){
            found = ind;
        } else if (ind == -1){
            return found;
        }
    }
}

void process_cmd(void){
    parse_cmd();
    if(cmd1_len == 0){
        
    } else if(strcmp(cmd1,"ls") == 0){
        if(cmd2_len != 0){
            sys_write("Too many arguments\n");
        } else {
            sys_ls();   
        }
    } else if(cmd2_len == 0 && strcmp(cmd1,"clean") == 0){
        if(cmd2_len != 0){
            sys_write("Too many arguments\n");
        } else {
            clear_screen();
            set_cursor(0);
        }
    } else if (cmd2_len == 0 && strcmp(cmd1,"help") == 0){
        if(cmd2_len != 0){
            sys_write("Too many arguments\n");
        } else {
            sys_write("Availables commands: ls help clean cd mkdir rm\n");
        }
    } else if (cmd2_len != 0 && cmd3_len == 0 && strcmp(cmd1,"mkdir") == 0) {
        if(cmd3_len != 0){
            sys_write("Too many arguments\n");
        } else {
            int ind = ind_of_file_in_current_directory(cmd2);
            if(ind != -1){
                sys_write(cmd2);
                sys_write(" already exists\n");
            } else {
                sys_mkdir();
            }
        }
    } else if (cmd2_len != 0 && cmd3_len == 0 && strcmp(cmd1,"cd") == 0){
        if(cmd3_len != 0){
            sys_write("Too many arguments\n");
        } else {
            int ind = ind_of_file_in_current_directory(cmd2);
            if(ind == -1){
                sys_write(cmd2); sys_write(" doesn't exist\n");
            } else {
                if(strcmp(cmd2,"..") == 0){
                    while(path_len > 0 && path[path_len] != '\\'){
                        path_len--;
                    }
                    path[path_len] = '\0';
                } else if(strcmp(cmd2,".") != 0){
                    path[path_len++] = '\\';
                    for(int i = 0; i<cmd2_len; i++){
                        path[path_len++] = cmd2[i];
                    }
                    path[path_len] = '\0';
                }
                sys_cd(ind);
            }
        }
    } else if (cmd2_len != 0 && cmd3_len == 0 && strcmp(cmd1,"rm") == 0){
        if(cmd3_len != 0){
            sys_write("Too many arguments\n");
        } else {
            int ind = ind_of_file_in_current_directory(cmd2);
            if(ind == -1){
                sys_write(cmd2); sys_write(" doesn't exist\n");
            } else {
                if(strcmp(cmd2,".") == 0 || strcmp(cmd2,"..") == 0){
                    sys_write(cmd2); sys_write(" can't be removed\n");
                } else {
                    sys_rm(ind);
                }
            }
        }
    } else {
        sys_write("Unknown command\n");
    }
    cmd_len = 0;
    cmd_ind = 0;
}

void sys_write_char(int pos, char c) {
    screen[pos] = c;
    asm volatile ("int $0x81"
        : 
        : "D"(c), "S"(pos)
        : "cc", "memory");
}

char sys_getchar() {
    char c;
    asm volatile ("int $0x80"
        : "=a"(c)
        :
        : "cc", "memory");
    return c;
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
            screen[l + j] = screen[l + j + 80];
            sys_write_char(l + j,screen[l + j]);
        }
    }
    for(int j = 0; j<80; j++){
        screen[24*80 + j] = 0;
        sys_write_char(24*80 + j,0);
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

void sys_ls(void){
    int ind = 0;
    while(ind != -1){
        asm  volatile ("int $0x83"
            : "=a"(ind)
            : "D"(file_name_return)
            : "cc", "memory");
        if(ind >= 0){
            sys_write("  ");
            sys_write(file_name_return);
        }
    }
    sys_write("\n");
}

void sys_mkdir(){
    asm volatile ("int $0x84"
        :
        : "D"(cmd2)
        : "cc", "memory");
}

void sys_cd(int ind){
    asm volatile ("int $0x85"
        :
        : "D"(ind)
        : "cc", "memory");
}

void sys_rm(int ind){
    asm volatile ("int $0x86"
        :
        : "D"(ind)
        : "cc", "memory");
}