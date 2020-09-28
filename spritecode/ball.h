#ifndef BALL_H
#define BALL_H
#include "../sprite.h"

typedef enum {
    BALL_STATE_INIT = 0,
    BALL_STATE_NORMAL,
} BALL_STATE;

void ball_make(int char_num, Fixed32 x, Fixed32 y);
void ball_move(SPRITE_INFO *ball);
#endif
