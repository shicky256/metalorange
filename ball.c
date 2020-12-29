#include <SEGA_MTH.H>

#include "ball.h"
#include "game.h"
#include "level.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "vblank.h"

BALL_SPRITE ball_sprites[MAX_BALLS];
int ball_count;

int ball_mode;

static int ball_charno;

void ball_init(int charno) {
    ball_charno = charno;
    ball_count = 0;
}

void ball_add(Fixed32 x_pos, Fixed32 y_pos, Fixed32 angle) {
    if (ball_count >= MAX_BALLS) {
        return;
    }
    //if spawning the first ball, attach it to the ship
    if (ball_count == 0) {
        //spawn ball offscreen
        ball_sprites[ball_count].x = MTH_FIXED(-80);
        ball_sprites[ball_count].y = MTH_FIXED(-80);
        ball_sprites[ball_count].speed = BALL_MINSPEED;
        ball_sprites[ball_count].angle = MTH_FIXED(45);
        ball_sprites[ball_count].state = BALL_STATE_INIT; //ball attached to ship
    }
    else {
        ball_sprites[ball_count].x = x_pos;
        ball_sprites[ball_count].y = y_pos;
        ball_sprites[ball_count].speed = BALL_MINSPEED;
        ball_sprites[ball_count].angle = angle;
        ball_sprites[ball_count].state = BALL_STATE_NORMAL; //ball comes out of ship
    }
    ball_sprites[ball_count].index = ball_count;
    ball_count++;
}

void ball_remove(BALL_SPRITE *ball) {
    int index = ball->index;
    ball_count--;
    //if we're not removing the last ball and  we have multiple balls
    if ((ball_count != index) && (ball_count > 0)) {
        ball_sprites[index] = ball_sprites[ball_count];
        ball_sprites[index].index = index;
    }
}

void ball_bounce(BALL_SPRITE *ball, int direction, int breakable) {
    // gigaball goes through breakable blocks
    if ((ball_mode == BALL_GIGABALL) && breakable) {
        return;
    }

    switch(direction) {
        case DIR_UP:
            if (ball->angle >= MTH_FIXED(0)) {
                ball->angle = -ball->angle;
            }
            break;

        case DIR_DOWN:
        // gigaball lets ball go through breakable blocks without rebounding
            if (ball->angle <= MTH_FIXED(0)) {
                ball->angle = -ball->angle;

            }
            break;

        case DIR_LEFT:
            if (ball->angle >= MTH_FIXED(90)) {
                ball->angle = MTH_FIXED(180) - ball->angle;
            }
            else if (ball->angle <= MTH_FIXED(-90)) {
                ball->angle = MTH_FIXED(-180) - ball->angle;
            }
            break;

        case DIR_RIGHT:
            if ((ball->angle >= MTH_FIXED(0)) && (ball->angle <= MTH_FIXED(90))) {
                ball->angle = MTH_FIXED(180) - ball->angle;
            }
            else if ((ball->angle <= MTH_FIXED(0)) && (ball->angle >= MTH_FIXED(-90))) {
                ball->angle = MTH_FIXED(-180) - ball->angle;
            }
            break;
    }
}

void ball_move() {
    for (int i = 0; i < ball_count; i++) {
        //when the player spawns, keep the ball attached to his ship
        //until he presses A
        if (ball_sprites[i].state == BALL_STATE_INIT) {
            ball_sprites[i].x = ship_sprite.x + BALL_SPAWN_XOFFSET;
            ball_sprites[i].y = ship_sprite.y + BALL_SPAWN_YOFFSET;
            // ball goes left when player goes left
            if (ship_sprite.dx < 0) {
                ball_sprites[i].angle = MTH_FIXED(135);
                // ball_sprites[i].dx = -ball_sprites[i].dx;
            }

            //don't allow ball launch until level is done loading and ship is done
            if ((PadData1E & PAD_A) && (level_doneload()) && (ship_sprite.state == SHIP_STATE_NORM)) {
                ball_sprites[i].state = BALL_STATE_NORMAL;
            }
        }
        else if (ball_sprites[i].state == BALL_STATE_NORMAL) {
            ball_sprites[i].x += MTH_Mul(MTH_Cos(ball_sprites[i].angle), ball_sprites[i].speed);
            ball_sprites[i].y += MTH_Mul(MTH_Sin(ball_sprites[i].angle), -ball_sprites[i].speed);
            //left/right wall
            if (ball_sprites[i].x <= BALL_LBOUND) {
                ball_bounce(&ball_sprites[i], DIR_LEFT, 0);
                ball_sprites[i].x = BALL_LBOUND + MTH_FIXED(1);
            }
            else if (ball_sprites[i].x >= BALL_RBOUND) {
                ball_bounce(&ball_sprites[i], DIR_RIGHT, 0);
                ball_sprites[i].x = BALL_RBOUND - MTH_FIXED(1);
            }
            //top of screen
            if (ball_sprites[i].y <= BALL_MARGIN) {
                ball_bounce(&ball_sprites[i], DIR_UP, 0);
                ball_sprites[i].y = BALL_MARGIN + MTH_FIXED(1);
            }
            //bottom
            if ((ball_sprites[i].x + BALL_WIDTH + BALL_MARGIN >= ship_left) &&
                (ball_sprites[i].x - BALL_MARGIN <= ship_right) && 
                (ball_sprites[i].y + BALL_HEIGHT - BALL_MARGIN >= ship_sprite.y + SHIP_YMARGIN) &&
                (ball_sprites[i].y + BALL_MARGIN < ship_sprite.y + SHIP_HEIGHT)) {
                // calculate ball's offset from paddle center
                Fixed32 ball_offset = (ship_sprite.x + (SHIP_WIDTH >> 1)) - (ball_sprites[i].x + (BALL_WIDTH >> 1));
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
                ball_sprites[i].angle = ball_angle;
                if (ball_sprites[i].speed < BALL_MAXSPEED) {
                    ball_sprites[i].speed += BALL_ACCEL;
                }
                sound_play(SOUND_SHIP); // play sound effect when ball hits ship
            }

            if (ball_sprites[i].y > MTH_FIXED(SCROLL_LORES_Y)) {
                ball_remove(&ball_sprites[i]);
                if (ball_count == 0) {
                    game_loss();
                }
                else { //rerun the ball code for the ball that's been swapped into this one's place
                    i--;
                }
            }
        }
    }
}

void ball_draw() {
    SPRITE_INFO ball_sprite;
    for (int i = 0; i < ball_count; i++) {
        sprite_make(ball_charno + ball_mode, ball_sprites[i].x, ball_sprites[i].y, &ball_sprite);
        sprite_draw(&ball_sprite);
    }
}
