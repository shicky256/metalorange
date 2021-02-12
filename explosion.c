#include <assert.h>
#include <sega_mth.h>

#include "explosion.h"
#include "sprite.h"
#include "print.h"

typedef struct {
    int anim_timer;
    int anim_cursor;
} EXPLOSION_DATA;

static_assert(sizeof(EXPLOSION_DATA) <= SPRITE_DATA_SIZE, "Struct size too large");


//how many frames to wait before switching to next frame
#define FRAME_DELAY (5)
#define NUM_FRAMES (4)

void explosion_make(int char_num, Fixed32 x, Fixed32 y) {
    SPRITE_INFO *expspr = sprite_next();
    sprite_make(char_num, x, y, expspr);
    EXPLOSION_DATA *expdata = (EXPLOSION_DATA *)expspr->data;
    expdata->anim_timer = FRAME_DELAY;
    expdata->anim_cursor = 0;
    expspr->iterate = &explosion_move;
}

void explosion_move(SPRITE_INFO *expspr) {
    EXPLOSION_DATA *expdata = (EXPLOSION_DATA *)expspr->data;
    if (expdata->anim_timer == 0) {
        expdata->anim_cursor++;
        expspr->char_num++;
        if (expdata->anim_cursor >= NUM_FRAMES) {
            sprite_delete(expspr);
            return;
        }
        expdata->anim_timer = FRAME_DELAY;
    }
    else {
        expdata->anim_timer--;
    }
}
