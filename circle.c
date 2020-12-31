#include <SEGA_MTH.H>

#include "capsule.h"
#include "circle.h"
#include "game.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"

#define CIRCLE_LBOUND (LEFT_WALL)
#define CIRCLE_RBOUND (RIGHT_WALL - CIRCLE_WIDTH)

#define CIRCLE_LEN (2)
CIRCLE circle_sprites[CIRCLE_LEN];
int circle_cursor;

int circle_charno;
// last normal frame
#define CIRCLE_LASTNORM (circle_charno + 2)
#define CIRCLE_EXPLOSION (circle_charno + 3)
#define CIRCLE_LASTEXP (circle_charno + 4)
#define CIRCLE_TIMING (10)

void circle_init(int charno) {
    circle_charno = charno;
    circle_cursor = 0;
}

void circle_add(Fixed32 x, Fixed32 y) {
    if (circle_cursor >= CIRCLE_LEN) {
        return;
    }

    circle_sprites[circle_cursor].x = x;
    circle_sprites[circle_cursor].y = y;
    circle_sprites[circle_cursor].index = circle_cursor;
    circle_sprites[circle_cursor].charno = circle_charno;
    circle_sprites[circle_cursor].movement_timer = 0;
    circle_sprites[circle_cursor].timer = 0;

    circle_cursor++;
}

void circle_remove(CIRCLE *circle) {
    int index = circle->index;
    circle_cursor--;
    // if deleting the last element, do nothing
    if (index == circle_cursor) {
        return;
    }
    // otherwise swap the deleting element and the last element
    circle_sprites[index] = circle_sprites[circle_cursor];
}

void circle_explode(CIRCLE *circle) {
    // only explode once
    if (circle->charno < CIRCLE_EXPLOSION) {
        circle->charno = CIRCLE_EXPLOSION;
        circle->timer = 0;
        sound_play(SOUND_CIRCLE);
    }
}

void circle_bounce(CIRCLE *circle, int direction) {
    switch (direction) {
        case DIR_UP:
            if (circle->angle >= MTH_FIXED(0)) {
                circle->angle = -circle->angle;
            }
        break;

        case DIR_DOWN:
            if (circle->angle <= MTH_FIXED(0)) {
                circle->angle = -circle->angle;
            }
        break;

        case DIR_LEFT:
            if (circle->angle >= MTH_FIXED(90)) {
                circle->angle = MTH_FIXED(180) - circle->angle;
            }
            else if (circle->angle <= MTH_FIXED(-90)) {
                circle->angle = MTH_FIXED(-180) - circle->angle;
            }
        break;

        case DIR_RIGHT:
            if ((circle->angle >= MTH_FIXED(0)) && (circle->angle <= MTH_FIXED(90))) {
                circle->angle = MTH_FIXED(180) - circle->angle;
            }
            else if ((circle->angle <= MTH_FIXED(0)) && (circle->angle >= MTH_FIXED(-90))) {
                circle->angle = MTH_FIXED(-180) - circle->angle;
            }
        break;
    }
}

void circle_move() {
    for (int i = 0; i < circle_cursor; i++) {
        if (circle_sprites[i].charno < CIRCLE_EXPLOSION) {
            if (circle_sprites[i].movement_timer == 0) {
                circle_sprites[i].movement_timer = 60;
                Fixed32 pos_rand = (Fixed32)(MTH_GetRand() & 0x7fffffff);
                circle_sprites[i].angle = (pos_rand % MTH_FIXED(360)) - MTH_FIXED(180);
            }
            else {
                circle_sprites[i].movement_timer--;
                circle_sprites[i].x += MTH_Mul(MTH_Cos(circle_sprites[i].angle), CIRCLE_SPEED);
                circle_sprites[i].y += MTH_Mul(MTH_Sin(circle_sprites[i].angle), -CIRCLE_SPEED);
            }
            // collision detection
            if (circle_sprites[i].x < CIRCLE_LBOUND) {
                circle_bounce(&circle_sprites[i], DIR_LEFT);
            }
            if (circle_sprites[i].x > CIRCLE_RBOUND) {
                circle_bounce(&circle_sprites[i], DIR_RIGHT);
            }
            if (circle_sprites[i].y < 0) {
                circle_bounce(&circle_sprites[i], DIR_UP);
            }
            if (circle_sprites[i].y > MTH_FIXED(180)) {
                circle_bounce(&circle_sprites[i], DIR_DOWN);
            }
        }

        circle_sprites[i].timer++;
        if (circle_sprites[i].timer >= CIRCLE_TIMING) {
            circle_sprites[i].timer = 0;
            if (circle_sprites[i].charno == CIRCLE_LASTNORM) {
                circle_sprites[i].charno = circle_charno;
            }
            else if (circle_sprites[i].charno == CIRCLE_LASTEXP) {
                capsule_add(circle_sprites[i].x, circle_sprites[i].y);
                circle_remove(&circle_sprites[i]);
                i--;
                continue;
            }
            else {
                circle_sprites[i].charno++;
            }
        }
    }
}

void circle_draw() {
    SPRITE_INFO sprite;
    // CIRCLE *circle;
    for (int i = 0; i < circle_cursor; i++) {
        sprite_make(circle_sprites[i].charno, circle_sprites[i].x, circle_sprites[i].y, &sprite);
        sprite_draw(&sprite);
    }
}
