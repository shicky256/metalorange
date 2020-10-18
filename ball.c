#include <SEGA_MTH.H>

#include "ball.h"
#include "game.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

SPRITE_INFO *ball_sprites[MAX_BALLS];
int ball_count;

static int ball_charno;

void ball_init(int charno) {
    ball_charno = charno;
    ball_count = 0;
}

void ball_add(Fixed32 x_pos, Fixed32 y_pos) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (ball_sprites[i] == NULL) {
            ball_sprites[i] = sprite_next();
            ball_count++;
            //if spawning the first ball, attach it to the ship
            if (ball_count == 1) {
                //spawn ball offscreen
                sprite_make(ball_charno, MTH_FIXED(-80), MTH_FIXED(-80), ball_sprites[i]);
                ball_sprites[i]->state = BALL_STATE_INIT; //ball attached to ship
            }
            else {
                sprite_make(ball_charno, x_pos, y_pos, ball_sprites[i]);
                ball_sprites[i]->dx = BALL_SPEED;
                ball_sprites[i]->dy = -BALL_SPEED;
                ball_sprites[i]->state = BALL_STATE_NORMAL; //ball comes out of ship
            }
            break;
        }
    }
}

void ball_bounce(SPRITE_INFO *ball, int direction) {
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
    for (int i = 0; i < MAX_BALLS; i++) {
        if (ball_sprites[i] != NULL) {
            //when the player spawns, keep the ball attached to his ship
            //until he presses A
            if (ball_sprites[i]->state == BALL_STATE_INIT) {
                ball_sprites[i]->x = ship_sprite->x + BALL_SPAWN_XOFFSET;
                ball_sprites[i]->y = ship_sprite->y + BALL_SPAWN_YOFFSET;
                if (ship_sprite->dx < 0) {
                    ball_sprites[i]->dx = -BALL_SPEED;
                    ball_sprites[i]->dy = -BALL_SPEED;
                }
                else {
                    ball_sprites[i]->dx = BALL_SPEED;
                    ball_sprites[i]->dy = -BALL_SPEED;
                }

                //don't allow ball launch until ship is done
                if ((PadData1E & PAD_A) && (ship_sprite->state == SHIP_STATE_NORM)) {
                    ball_sprites[i]->state = BALL_STATE_NORMAL;
                }
            }
            else if (ball_sprites[i]->state == BALL_STATE_NORMAL) {
                ball_sprites[i]->x += ball_sprites[i]->dx;
                ball_sprites[i]->y += ball_sprites[i]->dy;
                //left/right wall
                if ((ball_sprites[i]->x <= BALL_LBOUND) || ball_sprites[i]->x >= BALL_RBOUND) {
                    ball_sprites[i]->dx = -ball_sprites[i]->dx;
                }
                //top of screen
                if (ball_sprites[i]->y <= BALL_MARGIN) {
                    ball_sprites[i]->dy = -ball_sprites[i]->dy;
                }
                //bottom
                if ((ball_sprites[i]->x + BALL_WIDTH + BALL_MARGIN >= ship_sprite->x + SHIP_XMARGIN) &&
                    (ball_sprites[i]->x - BALL_MARGIN <= ship_sprite->x + SHIP_WIDTH - SHIP_XMARGIN) && 
                    (ball_sprites[i]->y + BALL_HEIGHT - BALL_MARGIN >= ship_sprite->y + SHIP_YMARGIN) &&
                    (ball_sprites[i]->y + BALL_MARGIN < ship_sprite->y + SHIP_HEIGHT)) {
                    //calculate ball's offset from paddle center
                    Fixed32 ball_offset = (ball_sprites[i]->x + (BALL_WIDTH >> 1)) - (ship_sprite->x + (SHIP_WIDTH >> 1));
                    Fixed32 normalized_offset = MTH_Div(ball_offset, (SHIP_WIDTH >> 1));
                    Fixed32 ball_angle = MTH_Mul(normalized_offset, MTH_FIXED(65));
                    ball_sprites[i]->dx = MTH_Mul(MTH_Sin(ball_angle), BALL_SPEED);
                    ball_sprites[i]->dy = MTH_Mul(MTH_Cos(ball_angle), -BALL_SPEED);
                }

                if (ball_sprites[i]->y > MTH_FIXED(SCROLL_LORES_Y)) {
                    sprite_delete(ball_sprites[i]);
                    ball_sprites[i] = NULL;
                    ball_count--;

                    if (ball_count == 0) {
                        game_loss();
                    }
                }
            }
        }
    }
}
