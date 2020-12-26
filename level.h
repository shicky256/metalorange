#ifndef LEVEL_H
#define LEVEL_H

extern int level_blocksleft;

//loads level
//base: sprite number of first block
//num: level number
void level_load(Uint16 base, int num);
//shows level
void level_disp();

#endif
