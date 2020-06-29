#include <sega_mth.h>

#include "explosion.h"
#include "../graphicrefs.h"
#include "../sprite.h"
#include "../sound.h"
#include "../player.h"
#include "../print.h"

#define EXPLOSION_MINCHARNUM (GRAPHIC_EXPLOSION)
#define EXPLOSION_MAXCHARNUM (GRAPHIC_EXPLOSION + explosion_num)
//how many frames to wait before switching to next frame
#define FRAME_DELAY (5)

void explosion_make(Fixed32 x, Fixed32 y) {
    SPRITE_INFO *explosion = sprite_next();
    sprite_make(EXPLOSION_MINCHARNUM, x, y, explosion);
    explosion->animTimer = FRAME_DELAY;
    explosion->iterate = &explosion_move;
    sound_play(SOUND_EXPLOSION);
}

void explosion_move(SPRITE_INFO *explosion) {
    if (explosion->animTimer == 0) {
        explosion->char_num++;
        if (explosion->char_num >= EXPLOSION_MAXCHARNUM) {
            sprite_delete(explosion);
            return;
        }
        explosion->animTimer = FRAME_DELAY;
    }
    else {
        explosion->animTimer--;
    }
}
