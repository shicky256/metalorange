#ifndef CAPSULE_H
#define CAPSULE_H

#define CAPSULE_MAX (10)

#define CAPSULE_WIDTH (MTH_FIXED(16))
#define CAPSULE_BOTTOM (MTH_FIXED(9))

// number of times out of 10 that a capsule appears
#define CAPSULE_PROBABILITY (1)

typedef struct {
    Fixed32 x;
    Fixed32 y;
    int charno;
    int anim_timer;
} CAPSULE_SPRITE;

void capsule_init(int charno);
//adds a capsule onscreen
void capsule_add(Fixed32 x, Fixed32 y);
//moves all the capsules
void capsule_run();
void capsule_draw();

#endif
