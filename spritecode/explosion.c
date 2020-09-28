#include <sega_mth.h>

#include "explosion.h"
#include "../graphicrefs.h"
#include "../sprite.h"
#include "../print.h"

//how many frames to wait before switching to next frame
#define FRAME_DELAY (5)
#define NUM_FRAMES (4)

void explosion_make(int char_num, Fixed32 x, Fixed32 y) {
    SPRITE_INFO *explosion = sprite_next();
    sprite_make(char_num, x, y, explosion);
    explosion->anim_timer = FRAME_DELAY;
    explosion->anim_cursor = 0;
    explosion->iterate = &explosion_move;
}

void explosion_move(SPRITE_INFO *explosion) {
    if (explosion->anim_timer == 0) {
        explosion->anim_cursor++;
        explosion->char_num++;
        if (explosion->anim_cursor >= NUM_FRAMES) {
            sprite_delete(explosion);
            return;
        }
        explosion->anim_timer = FRAME_DELAY;
    }
    else {
        explosion->anim_timer--;
    }
}
