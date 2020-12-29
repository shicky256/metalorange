#ifndef LEVEL_H
#define LEVEL_H

// number of breakable blocks left in the current level
extern int level_blocksleft;
// number of levels
extern int level_count;

//loads level
//base: sprite number of first block
//num: level number
void level_load(Uint16 base, int num);
// returns 1 if the level is fully loaded onscreen
int level_doneload();
//shows level
void level_disp();

#endif
