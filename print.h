#ifndef PRINT_H
#define PRINT_H
void print_load(); //load font gfx into sprite ram
void print_init();
void print_num(Uint32 num, int row, int col);
void print_string(char *ch, int row, int col);
void print_display();
#endif
