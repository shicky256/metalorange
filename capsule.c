#include <SEGA_MTH.H>

#include "capsule.h"
#include "game.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sprite.h"

//first capsule character in vdp1
static int capsule_charno;
//how many capsules there are
#define CAPSULE_CHARCOUNT (3)
//how fast the capsule moves
#define CAPSULE_SPEED (MTH_FIXED(1))
CAPSULE_SPRITE capsules[CAPSULE_MAX];
static int capsule_count;
#define CAPSULE_TIMING (16)

void capsule_init(int charno) {
    capsule_charno = charno;
    capsule_count = 0;
}

void capsule_add(Fixed32 x, Fixed32 y) {
    if (capsule_count < CAPSULE_MAX) {
        capsules[capsule_count].x = x;
        capsules[capsule_count].y = y;
        capsules[capsule_count].charno = capsule_charno;
        capsules[capsule_count].anim_timer = 0;
        capsule_count++;
    }
}

void capsule_remove(int num) {
    capsule_count--;
    if ((capsule_count != num) && (capsule_count > 0)) {
        capsules[num] = capsules[capsule_count];
    }
}

void capsule_run() {
    SPRITE_INFO spr;
    CAPSULE_SPRITE *capsule;
    for (int i = 0; i < capsule_count; i++) {
        capsule = &(capsules[i]);
        //make the capsule fall
        capsule->y += CAPSULE_SPEED;
        //animate it
        capsule->anim_timer++;
        if (capsule->anim_timer > CAPSULE_TIMING) {
            capsule->anim_timer = 0;
            capsule->charno++;
            if (capsule->charno - capsule_charno >= CAPSULE_CHARCOUNT) {
                capsule->charno = capsule_charno;
            }
        }
        int collision = 0;
        if ((capsule->x + CAPSULE_WIDTH >= ship_left) &&
            (capsule->x <= ship_right) && 
            (capsule->y + CAPSULE_BOTTOM >= ship_sprite->y + SHIP_YMARGIN) &&
            (capsule->y < ship_sprite->y + SHIP_HEIGHT)) {
            collision = 1;
            game_incpowerup();
        }
        //remove the capsule if it's passed the bottom of the screen or is touching the player
        if ((capsule->y > MTH_FIXED(SCROLL_LORES_Y) || collision)) {
            capsule_remove(i);
            //after the switch, repeat for the capsule that's been moved into this one's place
            i--;
        }
        // otherwise display the capsule
        else {
            sprite_make(capsule->charno, capsule->x, capsule->y, &spr);
            sprite_draw(&spr);
        }
    }
}
