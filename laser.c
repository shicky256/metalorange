#include <assert.h>
#include <sega_mth.h>
#include "game.h"
#include "laser.h"
#include "sprite.h"

#define LASER_SPEED (MTH_FIXED(-4))
#define LASER_HEIGHT (MTH_FIXED(6))

SPRITE_INFO *laser_head;

int laser_count;
static int laser_charno;

void laser_init(int charno) {
    laser_charno = charno;
    laser_count = 0;
    laser_head = NULL;
}

static void laser_move(SPRITE_INFO *laser) {
    if (!game_playing()) {
        return;
    }
    laser->y += LASER_SPEED; 
    if (laser->y < 0) { // remove laser when it goes off the screen
        laser_remove(laser);
    }
}

void laser_add(Fixed32 x, Fixed32 y) {
    if (laser_count > MAX_LASERS) {
        return;
    }
    SPRITE_INFO *laser = sprite_next();
    sprite_make(laser_charno, x, y, laser);
    sprite_listadd(&laser_head, laser);
    laser->iterate = laser_move;
    laser_count++;
}

void laser_remove(SPRITE_INFO *laser) {
    sprite_listremove(&laser_head, laser);
    laser_count--;
    sprite_delete(laser);
}

void laser_removeall() {
    while (laser_head) {
        laser_remove(laser_head);
    }
}
