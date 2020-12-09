#include <SEGA_MTH.H>

#include "ball.h"
#include "game.h"
#include "print.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

BALL_SPRITE ball_sprites[MAX_BALLS];
int ball_count;

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
        ball_sprites[ball_count].dx = MTH_Mul(MTH_Cos(angle), BALL_SPEED);
        ball_sprites[ball_count].dy = MTH_Mul(MTH_Sin(angle), -BALL_SPEED);
        ball_sprites[ball_count].state = BALL_STATE_INIT; //ball attached to ship
    }
    else {
        ball_sprites[ball_count].x = x_pos;
        ball_sprites[ball_count].y = y_pos;
        ball_sprites[ball_count].dx = MTH_Mul(MTH_Cos(angle), BALL_SPEED);
        ball_sprites[ball_count].dy = MTH_Mul(MTH_Sin(angle), -BALL_SPEED);
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

void ball_bounce(BALL_SPRITE *ball, int direction) {
    switch(direction) {
        case DIR_UP:
        case DIR_DOWN:
            ball->dy = -ball->dy;
            break;

        case DIR_LEFT:
        case DIR_RIGHT:
            ball->dx = -ball->dx;
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
                ball_sprites[i].dx = -ball_sprites[i].dx;
            }

            //don't allow ball launch until ship is done
            if ((PadData1E & PAD_A) && (ship_sprite.state == SHIP_STATE_NORM)) {
                ball_sprites[i].state = BALL_STATE_NORMAL;
            }
        }
        else if (ball_sprites[i].state == BALL_STATE_NORMAL) {
            ball_sprites[i].x += ball_sprites[i].dx;
            ball_sprites[i].y += ball_sprites[i].dy;
            //left/right wall
            if (ball_sprites[i].x <= BALL_LBOUND) {
                ball_sprites[i].dx = -ball_sprites[i].dx;
                ball_sprites[i].x = BALL_LBOUND + MTH_FIXED(1);
            }
            else if (ball_sprites[i].x >= BALL_RBOUND) {
                ball_sprites[i].dx = -ball_sprites[i].dx;
                ball_sprites[i].x = BALL_RBOUND - MTH_FIXED(1);
            }
            //top of screen
            if (ball_sprites[i].y <= BALL_MARGIN) {
                ball_sprites[i].dy = -ball_sprites[i].dy;
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
                ball_sprites[i].dx = MTH_Mul(MTH_Cos(ball_angle), BALL_SPEED);
                ball_sprites[i].dy = MTH_Mul(MTH_Sin(ball_angle), -BALL_SPEED);
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
        sprite_make(ball_charno, ball_sprites[i].x, ball_sprites[i].y, &ball_sprite);
        sprite_draw(&ball_sprite);
    }
}
