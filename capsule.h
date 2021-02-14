#ifndef CAPSULE_H
#define CAPSULE_H

#define CAPSULE_WIDTH (MTH_FIXED(16))
#define CAPSULE_BOTTOM (MTH_FIXED(9))

// number of times out of 10 that a capsule appears
#define CAPSULE_PROBABILITY (1)

void capsule_init(int charno);
//adds a capsule onscreen
void capsule_add(Fixed32 x, Fixed32 y);
void capsule_removeall();
#endif
