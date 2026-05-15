#include "syscalls.h"

void process_main(){
    sys_call();
    while (1) {}
        /*sys_write("> ");
        sys_get_input(cmd);

        if (!strcmp(cmd, "ls")){
            sys_ls();
        } else if (!strcmp(cmd, "help")){
            sys_help();
        } else if (!strcmp(cmd, "clean")){
            sys_clean();
        } else {
            sys_write("Unknown command\n");
        }*/
}