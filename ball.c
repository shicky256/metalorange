#include <SEGA_MTH.H>

#include "ball.h"
#include "game.h"
#include "level.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "vblank.h"

int ball_count;

int ball_mode;

SPRITE_INFO *ball_head;

static int ball_charno;

#define NORMAL_CHARNO (ball_charno)
#define GIGABALL_CHARNO (ball_charno + 1)

void ball_init(int charno) {
    ball_charno = charno;
    ball_count = 0;
    ball_head = NULL;
}

static void ball_remove(SPRITE_INFO *ball) {
    sprite_listremove(&ball_head, ball);
    ball_count--;
    sprite_delete(ball);
}

void ball_removeall() {
    while (ball_head) {
        ball_remove(ball_head);
    }
}

static void ball_move(SPRITE_INFO *ball) {
    BALL_DATA *ball_data = (BALL_DATA *)ball->data;
    if (!game_playing()) {
        return;
    }

    if (ball_mode == BALL_GIGABALL) {
        ball->char_num = GIGABALL_CHARNO;
    }
    else {
        ball->char_num = NORMAL_CHARNO;
    }

    if (ball_data->state == BALL_STATE_INIT) {
        ball->x = ship_sprite.x + BALL_SPAWN_XOFFSET;
        ball->y = ship_sprite.y + BALL_SPAWN_YOFFSET;
        // ball goes left when player goes left
        if (ship_sprite.dx < 0) {
            ball_data->angle = MTH_FIXED(135);
        }

        //don't allow ball launch until level is done loading and ship is done
        if ((PadData1E & PAD_A) && (level_doneload()) && (ship_sprite.state == SHIP_STATE_NORM)) {
            ball_data->state = BALL_STATE_NORMAL;
        }
    }
    else if (ball_data->state == BALL_STATE_NORMAL) {
        ball->x += MTH_Mul(MTH_Cos(ball_data->angle), ball_data->speed);
        ball->y += MTH_Mul(MTH_Sin(ball_data->angle), -ball_data->speed);
        //left/right wall
        if (ball->x <= BALL_LBOUND) {
            ball_bounce(ball, DIR_LEFT, 0);
            ball->x = BALL_LBOUND + MTH_FIXED(1);
        }
        else if (ball->x >= BALL_RBOUND) {
            ball_bounce(ball, DIR_RIGHT, 0);
            ball->x = BALL_RBOUND - MTH_FIXED(1);
        }
        //top of screen
        if (ball->y <= BALL_MARGIN) {
            ball_bounce(ball, DIR_UP, 0);
            ball->y = BALL_MARGIN + MTH_FIXED(1);
        }
        //bottom
        if ((ball->x + BALL_WIDTH + BALL_MARGIN >= ship_left) &&
            (ball->x - BALL_MARGIN <= ship_right) && 
            (ball->y + BALL_HEIGHT - BALL_MARGIN >= ship_sprite.y + SHIP_YMARGIN) &&
            (ball->y + BALL_MARGIN < ship_sprite.y + SHIP_HEIGHT)) {
            // calculate ball's offset from paddle center
            Fixed32 ball_offset = (ship_sprite.x + (SHIP_WIDTH >> 1)) - (ball->x + (BALL_WIDTH >> 1));
            // fraction from -1 to 1
            Fixed32 normalized_offset = MTH_Div(ball_offset, (SHIP_WIDTH >> 1));
            // we want an angle range of 90 degrees +/- 65
            Fixed32 ball_angle = MTH_Mul(normalized_offset, MTH_FIXED(65));
            ball_angle += MTH_FIXED(90);
            if (ball_angle < MTH_FIXED(25)) {
                ball_angle = MTH_FIXED(25);
            }
            if (ball_angle > MTH_FIXED(155)) {
                ball_angle = MTH_FIXED(155);
            }
            ball_data->angle = ball_angle;
            if (ball_data->speed < BALL_MAXSPEED) {
                ball_data->speed += BALL_ACCEL;
            }
            sound_play(SOUND_SHIP); // play sound effect when ball hits ship
        }

        if (ball->y > MTH_FIXED(SCROLL_LORES_Y)) {
            ball_remove(ball);
            if (ball_count == 0) {
                game_loss();
            }
        }
    }
}

void ball_add(Fixed32 x, Fixed32 y, Fixed32 angle) {
    SPRITE_INFO *ball = sprite_next();
    sprite_make(ball_charno, x, y, ball);
    BALL_DATA *ball_data = (BALL_DATA *)ball->data;
    ball->iterate = ball_move;
    //if spawning the first ball, attach it to the ship
    if (ball_count == 0) {
        //spawn ball offscreen
        ball->x = MTH_FIXED(-80);
        ball->y = MTH_FIXED(-80);
        ball_data->speed = BALL_MINSPEED;
        ball_data->angle = MTH_FIXED(45);
        ball_data->state = BALL_STATE_INIT; //ball attached to ship
    }
    else {
        ball_data->speed = BALL_MINSPEED;
        ball_data->angle = angle;
        ball_data->state = BALL_STATE_NORMAL; //ball comes out of ship
    }
    sprite_listadd(&ball_head, ball);
    ball_count++;
}

// this gets called by anything that can bounce off a ball
void ball_bounce(SPRITE_INFO *ball, int direction, int breakable) {
    BALL_DATA *ball_data = (BALL_DATA *)(ball->data);
    // gigaball goes through breakable blocks
    if ((ball_mode == BALL_GIGABALL) && breakable) {
        return;
    }

    switch(direction) {
        case DIR_UP:
            if (ball_data->angle >= MTH_FIXED(0)) {
                ball_data->angle = -ball_data->angle;
            }
            break;

        case DIR_DOWN:
            if (ball_data->angle <= MTH_FIXED(0)) {
                ball_data->angle = -ball_data->angle;

            }
            break;

        case DIR_LEFT:
            if (ball_data->angle >= MTH_FIXED(90)) {
                ball_data->angle = MTH_FIXED(180) - ball_data->angle;
            }
            else if (ball_data->angle <= MTH_FIXED(-90)) {
                ball_data->angle = MTH_FIXED(-180) - ball_data->angle;
            }
            break;

        case DIR_RIGHT:
            if ((ball_data->angle >= MTH_FIXED(0)) && (ball_data->angle <= MTH_FIXED(90))) {
                ball_data->angle = MTH_FIXED(180) - ball_data->angle;
            }
            else if ((ball_data->angle <= MTH_FIXED(0)) && (ball_data->angle >= MTH_FIXED(-90))) {
                ball_data->angle = MTH_FIXED(-180) - ball_data->angle;
            }
            break;
    }
}
