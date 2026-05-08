typedef struct console_line console_line;
struct console_line{
    int cursor;
    char content[80];	
};

void shell();
