#ifndef BALL_H
#define BALL_H
#include <assert.h>
#include "sprite.h"

#define BALL_MARGIN (MTH_FIXED(2))
#define BALL_WIDTH (MTH_FIXED(8))
#define BALL_HEIGHT (MTH_FIXED(8))

#define BALL_TSENSORX (BALL_MARGIN + (BALL_WIDTH >> 1))
#define BALL_TSENSORY (BALL_MARGIN)

#define BALL_BSENSORX (BALL_MARGIN + (BALL_WIDTH >> 1))
#define BALL_BSENSORY (BALL_MARGIN + BALL_HEIGHT)

#define BALL_LSENSORX (BALL_MARGIN)
#define BALL_LSENSORY (BALL_MARGIN + (BALL_HEIGHT >> 1))

#define BALL_RSENSORX (BALL_MARGIN + BALL_WIDTH)
#define BALL_RSENSORY (BALL_MARGIN + (BALL_HEIGHT >> 1))

#define BALL_MINSPEED (MTH_FIXED(3))
#define BALL_MAXSPEED (MTH_FIXED(4))
#define BALL_ACCEL (MTH_FIXED(0.02))

#define BALL_MAXANGLE (MTH_FIXED(65))
#define BALL_LBOUND (LEFT_WALL - BALL_MARGIN)
#define BALL_RBOUND (RIGHT_WALL - BALL_WIDTH + BALL_MARGIN)
#define BALL_STATE_INIT (0)
#define BALL_STATE_NORMAL (1)
#define BALL_SPAWN_XOFFSET (MTH_FIXED(16))
#define BALL_SPAWN_YOFFSET (MTH_FIXED(-2))

typedef struct {
    Fixed32 speed;
    Fixed32 angle;
    int state;
} BALL_DATA;

static_assert(sizeof(BALL_DATA) <= SPRITE_DATA_SIZE, "Struct size too large");

extern SPRITE_INFO *ball_head;

typedef enum {
    BALL_NORMAL = 0,
    BALL_GIGABALL
} BALL_MODE;

extern int ball_mode;

void ball_init(int charno);
void ball_add(Fixed32 x, Fixed32 y, Fixed32 angle);
void ball_removeall(void);
//makes a ball bounce from a collision in the given direction
#define DIR_UP (0)
#define DIR_DOWN (1)
#define DIR_LEFT (2)
#define DIR_RIGHT (3)
void ball_bounce(SPRITE_INFO *ball, int direction, int breakable);

#endif
