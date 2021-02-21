#include <sega_mth.h>

#include "ball.h"
#include "capsule.h"
#include "circle.h"
#include "game.h"
#include "laser.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"

#define CIRCLE_LBOUND (LEFT_WALL)
#define CIRCLE_RBOUND (RIGHT_WALL - CIRCLE_WIDTH)

#define MAX_CIRCLES (2)

void circle_move(SPRITE_INFO *circle);

int circle_count;

SPRITE_INFO *circle_head;

int circle_charno;
// last normal frame
#define CIRCLE_LASTNORM (circle_charno + 2)
#define CIRCLE_EXPLOSION (circle_charno + 3)
#define CIRCLE_LASTEXP (circle_charno + 4)
#define CIRCLE_TIMING (10)

void circle_init(int charno) {
    circle_charno = charno;
    circle_count = 0;
    circle_head = NULL;
}

void circle_add(Fixed32 x, Fixed32 y) {
    if (circle_count >= MAX_CIRCLES) {
        return;
    }

    SPRITE_INFO *circle = sprite_next();
    sprite_make(circle_charno, x, y, circle);
    circle->iterate = circle_move;
    CIRCLE_DATA *circle_data = (CIRCLE_DATA *)circle->data;
    circle_data->movement_timer = 0;
    circle_data->timer = 0;
    sprite_listadd(&circle_head, circle);
    circle_count++;
}

void circle_remove(SPRITE_INFO *circle) {
    sprite_listremove(&circle_head, circle);
    circle_count--;
    sprite_delete(circle);
}

void circle_removeall() {
    while (circle_head) {
        circle_remove(circle_head);
    }
}

void circle_explode(SPRITE_INFO *circle) {
    // only explode once
    if (circle->char_num < CIRCLE_EXPLOSION) {
        circle->char_num = CIRCLE_EXPLOSION;
        sound_play(SOUND_CIRCLE);
    }
}

void circle_bounce(SPRITE_INFO *circle, int direction) {
    CIRCLE_DATA *circle_data = (CIRCLE_DATA *)circle->data;
    switch (direction) {
        case DIR_UP:
            if (circle_data->angle >= MTH_FIXED(0)) {
                circle_data->angle = -circle_data->angle;
            }
        break;

        case DIR_DOWN:
            if (circle_data->angle <= MTH_FIXED(0)) {
                circle_data->angle = -circle_data->angle;
            }
        break;

        case DIR_LEFT:
            if (circle_data->angle >= MTH_FIXED(90)) {
                circle_data->angle = MTH_FIXED(180) - circle_data->angle;
            }
            else if (circle_data->angle <= MTH_FIXED(-90)) {
                circle_data->angle = MTH_FIXED(-180) - circle_data->angle;
            }
        break;

        case DIR_RIGHT:
            if ((circle_data->angle >= MTH_FIXED(0)) && (circle_data->angle <= MTH_FIXED(90))) {
                circle_data->angle = MTH_FIXED(180) - circle_data->angle;
            }
            else if ((circle_data->angle <= MTH_FIXED(0)) && (circle_data->angle >= MTH_FIXED(-90))) {
                circle_data->angle = MTH_FIXED(-180) - circle_data->angle;
            }
        break;
    }
}

void circle_move(SPRITE_INFO *circle) {
    if (!game_playing()) {
        return;
    }

    CIRCLE_DATA *circle_data = (CIRCLE_DATA *)circle->data;
    if (circle->char_num < CIRCLE_EXPLOSION) {
        if (circle_data->movement_timer == 0) {
            circle_data->movement_timer = 60;
            Fixed32 pos_rand = (Fixed32)(MTH_GetRand() & 0x7fffffff);
            circle_data->angle = (pos_rand % MTH_FIXED(360)) - MTH_FIXED(180);
        }
        else {
            circle_data->movement_timer--;
            circle->x += MTH_Mul(MTH_Cos(circle_data->angle), CIRCLE_SPEED);
            circle->y += MTH_Mul(MTH_Sin(circle_data->angle), -CIRCLE_SPEED);
        }
        // collision detection
        if (circle->x < CIRCLE_LBOUND) {
            circle_bounce(circle, DIR_LEFT);
        }
        if (circle->x > CIRCLE_RBOUND) {
            circle_bounce(circle, DIR_RIGHT);
        }
        if (circle->y < 0) {
            circle_bounce(circle, DIR_UP);
        }
        if (circle->y > MTH_FIXED(180)) {
            circle_bounce(circle, DIR_DOWN);
        }
        // check for balls
        SPRITE_INFO *ball = ball_head;
        while (ball != NULL) {
            if ((ball->x >= circle->x) && (ball->x < (circle->x + CIRCLE_WIDTH))) {
                if ((ball->y >= circle->y) && (ball->y < (circle->y + CIRCLE_HEIGHT))) {
                    circle_explode(circle);
                    break;
                }
            }
            ball = ball->next;
        }
        // check for lasers
        SPRITE_INFO *laser = laser_head;
        while (laser != NULL) {
            if ((laser->x >= circle->x) && (laser->x < (circle->x + CIRCLE_WIDTH))) {
                if ((laser->y >= circle->y) && (laser->y < (circle->y + CIRCLE_HEIGHT))) {
                    circle_explode(circle);
                    break;
                }
            }
            laser = laser->next;
        }
    }

    circle_data->timer++;
    if (circle_data->timer >= CIRCLE_TIMING) {
        circle_data->timer = 0;
        if (circle->char_num == CIRCLE_LASTNORM) {
            circle->char_num = circle_charno;
        }
        else if (circle->char_num == CIRCLE_LASTEXP) {
            capsule_add(circle->x, circle->y);
            circle_remove(circle);
        }
        else {
            circle->char_num++;
        }
    }
}
