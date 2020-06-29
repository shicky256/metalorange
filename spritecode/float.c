#include <sega_mth.h>

#include "float.h"
#include "explosion.h"
#include "../collision.h"
#include "../graphicrefs.h"
#include "../sprite.h"
#include "../player.h"

#define FLOAT_CHARNUM (GRAPHIC_FLOAT)
#define FLOAT_MAXSPEED (MTH_FIXED(1))
#define FLOAT_ACCEL (MTH_FIXED(0.05))
#define FLOAT_STATE_DOWN (0)
#define FLOAT_STATE_UP (1)
#define FLOAT_SIZE (MTH_FIXED(32))

void float_make(Fixed32 x, Fixed32 y) {
    SPRITE_INFO *float_spr = sprite_next();
    sprite_make(FLOAT_CHARNUM, x, y, float_spr);
    float_spr->xSize = FLOAT_SIZE;
    float_spr->ySize = FLOAT_SIZE;    
    float_spr->animTimer = 0;
    float_spr->iterate = &float_move;
    float_spr->state = FLOAT_STATE_DOWN;
}

void float_move(SPRITE_INFO *float_spr) {
    //make flickery flame
    if (float_spr->char_num == GRAPHIC_FLOAT) {
        float_spr->char_num++;
    }
    else {
        float_spr->char_num--;
    }

    if (float_spr->state == FLOAT_STATE_DOWN) {
        if (float_spr->dy > -FLOAT_MAXSPEED) {
            float_spr->dy -= FLOAT_ACCEL;
        }
        else {
            float_spr->dy = -FLOAT_MAXSPEED;
            float_spr->state = FLOAT_STATE_UP;
        }
    }
    else {
        if (float_spr->dy < FLOAT_MAXSPEED) {
            float_spr->dy += FLOAT_ACCEL;
        }
        else {
            float_spr->dy = FLOAT_MAXSPEED;
            float_spr->state = FLOAT_STATE_DOWN;
        }
    }
    float_spr->yPos += float_spr->dy;

    if (collision_player(float_spr)) {
            if (player_cankill()) {
                explosion_make(float_spr->xPos, float_spr->yPos);
                sprite_delete(float_spr);
                player_killenemy();
            }
            else {
                player_die();
            }
        }    
}
