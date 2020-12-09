#include <SEGA_MTH.H>
#include "laser.h"
#include "sprite.h"

#define LASER_SPEED (MTH_FIXED(-4))
#define LASER_HEIGHT (MTH_FIXED(6))

LASER_SPRITE laser_sprites[MAX_LASERS];
int laser_count;

static int laser_charno;

void laser_init(int charno) {
    laser_charno = charno;
    laser_count = 0;
}

void laser_add(Fixed32 x, Fixed32 y) {
    if (laser_count > MAX_LASERS) {
        return;
    }
    laser_sprites[laser_count].x = x;
    laser_sprites[laser_count].y = y;
    laser_sprites[laser_count].index = laser_count;
    laser_count++;
}

void laser_remove(LASER_SPRITE *sprite) {
    int index = sprite->index;
    laser_count--;
    // swap last sprite with current
    if ((laser_count != index) && (laser_count > 0)) {
        laser_sprites[index] = laser_sprites[laser_count];
        laser_sprites[index].index = index;
    }
}

void laser_move() {
    for (int i = 0; i < laser_count; i++) {
        laser_sprites[i].y += LASER_SPEED; 
        if (laser_sprites[i].y < 0) { // remove laser when it goes off the screen
            laser_remove(&laser_sprites[i]);
            i--; // re-run for the laser that's been swapped in this one's place
        }
    }
}

void laser_draw() {
    SPRITE_INFO laser_sprite;
    for (int i = 0; i < laser_count; i++) {
        sprite_make(laser_charno, laser_sprites[i].x, laser_sprites[i].y, &laser_sprite);
        sprite_draw(&laser_sprite);
    }
}
