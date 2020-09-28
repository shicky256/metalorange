#include <sega_mth.h>

#include "../game.h"
#include "../graphicrefs.h"
#include "../scroll.h"
#include "../sprite.h"
#include "../sound.h"
#include "../print.h"

#include "ball.h"

#define BALL_MARGIN (MTH_FIXED(2))
#define BALL_WIDTH (MTH_FIXED(8))
#define BALL_HEIGHT (MTH_FIXED(8))
#define BALL_SPEED (MTH_FIXED(3))
#define BALL_LBOUND (LEFT_WALL - BALL_MARGIN)
#define BALL_RBOUND (RIGHT_WALL - BALL_WIDTH + BALL_MARGIN)
#define BALL_STATE_INIT (0)
#define BALL_STATE_NORMAL (1)
#define BALL_SPAWN_XOFFSET (MTH_FIXED(16))
#define BALL_SPAWN_YOFFSET (MTH_FIXED(-2))

void ball_make(int char_num, Fixed32 x, Fixed32 y) {
    SPRITE_INFO *ball = sprite_next();
    sprite_make(char_num, x, y, ball);
    ball->iterate = &ball_move;
    ball->state = BALL_STATE_INIT;
}

void ball_move(SPRITE_INFO *ball) {
    //when the player spawns, keep the ball attached to his ship
    //until he presses A
    if (ball->state == BALL_STATE_INIT) {
        ball->x = ship_sprite->x + BALL_SPAWN_XOFFSET;
        ball->y = ship_sprite->y + BALL_SPAWN_YOFFSET;
        if (ship_sprite->dx < 0) {
            ball->dx = -BALL_SPEED;
            ball->dy = -BALL_SPEED;
        }
        else {
            ball->dx = BALL_SPEED;
            ball->dy = -BALL_SPEED;
        }
    }
    else if (ball->state == BALL_STATE_NORMAL) {
        ball->x += ball->dx;
        ball->y += ball->dy;
        //left/right wall
        if ((ball->x <= BALL_LBOUND) || ball->x >= BALL_RBOUND) {
            ball->dx = -ball->dx;
        }
        //top of screen
        if (ball->y <= BALL_MARGIN) {
            ball->dy = -ball->dy;
        }
        //bottom
        if ((ball->x + BALL_MARGIN >= ship_sprite->x + SHIP_XMARGIN) &&
            (ball->x + BALL_WIDTH - BALL_MARGIN <= ship_sprite->x + SHIP_WIDTH - SHIP_XMARGIN) && 
            (ball->y + BALL_HEIGHT - BALL_MARGIN >= ship_sprite->y + SHIP_YMARGIN) &&
            (ball->y + BALL_MARGIN < ship_sprite->y + SHIP_HEIGHT)) {
            //calculate ball's offset from paddle center
            Fixed32 ball_offset = (ball->x + (BALL_WIDTH >> 1)) - (ship_sprite->x + (SHIP_WIDTH >> 1));
            Fixed32 normalized_offset = MTH_Div(ball_offset, (SHIP_WIDTH >> 1));
            Fixed32 ball_angle = MTH_Mul(normalized_offset, MTH_FIXED(75));
            ball->dx = MTH_Mul(MTH_Sin(ball_angle), BALL_SPEED);
            ball->dy = MTH_Mul(MTH_Cos(ball_angle), -BALL_SPEED);
        }
    }
}
