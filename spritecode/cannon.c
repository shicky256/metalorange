#include <sega_mth.h>

#include "cannon.h"
#include "missile.h"
#include "../graphicrefs.h"
#include "../sprite.h"
#include "../sound.h"
#include "../player.h"
#include "../print.h"

#define CANNON_FIRE (150)

void cannon_make(Fixed32 x, Fixed32 y) {
    SPRITE_INFO *cannon = sprite_next();
    sprite_make(GRAPHIC_CANNON, x, y, cannon);
    cannon->iterate = &cannon_move;
    cannon->animTimer = CANNON_FIRE;
}

void cannon_move(SPRITE_INFO *cannon) {
    cannon->animTimer--;
    if (cannon->animTimer == 0) {
        missile_make(cannon->xPos, cannon->yPos);
        cannon->animTimer = CANNON_FIRE;
    }
    if (player.xPos > cannon->xPos) {
        cannon->mirror = MIRROR_HORIZ;
    }
    else {
        cannon->mirror = 0;
    }
}
