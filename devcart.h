#ifndef DEVCART_H
#define DEVCART_H

//loads file with filename specified from computer
int devcart_loadfile(Sint8 *filename, void *dest);
//prints string to computer
void devcart_printstr(char *string);
//reset back to file menu
void devcart_reset(void);

#endif
