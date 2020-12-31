#ifndef CIRCLE_H
#define CIRCLE_H

typedef struct {
    Fixed32 x;
    Fixed32 y;
    Fixed32 angle;
    int index;
    int charno;
    int timer;
    int movement_timer;
} CIRCLE;

#define CIRCLE_PXWIDTH (16)
#define CIRCLE_PXHEIGHT (16)

#define CIRCLE_WIDTH (MTH_FIXED(CIRCLE_PXWIDTH))
#define CIRCLE_HEIGHT (MTH_FIXED(CIRCLE_PXHEIGHT))
// sensor offsets
#define CIRCLE_TSENSORX (MTH_FIXED(CIRCLE_PXWIDTH / 2))
#define CIRCLE_TSENSORY (0)

#define CIRCLE_BSENSORX (MTH_FIXED(CIRCLE_PXWIDTH / 2))
#define CIRCLE_BSENSORY (CIRCLE_HEIGHT)

#define CIRCLE_LSENSORX (0)
#define CIRCLE_LSENSORY (MTH_FIXED(CIRCLE_PXHEIGHT / 2))

#define CIRCLE_RSENSORX (CIRCLE_WIDTH)
#define CIRCLE_RSENSORY (MTH_FIXED(CIRCLE_PXHEIGHT / 2))

#define CIRCLE_SPEED (MTH_FIXED(1))

extern CIRCLE circle_sprites[];
extern int circle_cursor;

void circle_init(int charno);
void circle_add(Fixed32 x, Fixed32 y);
void circle_explode(CIRCLE *circle);
#define DIR_UP (0)
#define DIR_DOWN (1)
#define DIR_LEFT (2)
#define DIR_RIGHT (3)
void circle_bounce(CIRCLE *circle, int direction);
void circle_move();
void circle_draw();

#endif
