#include <sega_mth.h>

#include "missile.h"
#include "explosion.h"
#include "../collision.h"
#include "../graphicrefs.h"
#include "../sprite.h"
#include "../player.h"

//max # of degrees missile can rotate in a frame
#define MISSILE_ROTATE_SPEED (MTH_FIXED(3)) 

#define MISSILE_SPEED (MTH_FIXED(2))

void missile_make(Fixed32 x, Fixed32 y) {
    SPRITE_INFO *missile = sprite_next();
    sprite_make(GRAPHIC_MISSILE, x, y, missile);
    missile->iterate = &missile_move;
    missile->xSize = MTH_FIXED(16);
    missile->ySize = MTH_FIXED(16);
    //start missile pointed at the player
    Fixed32 x_diff = (player.xPos + (player.xSize >> 1)) - (missile->xPos + (missile->xSize >> 1));
    Fixed32 y_diff = (player.yPos + (player.ySize >> 1)) - (missile->yPos + (missile->ySize >> 1));
    missile->angle = MTH_Atan(y_diff, x_diff);
}

void missile_move(SPRITE_INFO *missile) {
    Fixed32 x_diff = (player.xPos + (player.xSize >> 1)) - (missile->xPos + (missile->xSize >> 1));
    Fixed32 y_diff = (player.yPos + (player.ySize >> 1)) - (missile->yPos + (missile->ySize >> 1));
    Fixed32 target_angle = MTH_Atan(y_diff, x_diff);
    //handle when target angle goes over the x axis
    if (missile->angle > MTH_FIXED(90) && target_angle < MTH_FIXED(-90)) {
        missile->angle += MISSILE_ROTATE_SPEED;
    }
    else if (missile->angle < MTH_FIXED(-90) && target_angle > MTH_FIXED(90)) {
        missile->angle -= MISSILE_ROTATE_SPEED;
    }
    //make maximum speed the angle changes MISSILE_ROTATE_SPEED
    else if (target_angle - missile->angle < MISSILE_ROTATE_SPEED && target_angle - missile->angle > -MISSILE_ROTATE_SPEED) {
        missile->angle = target_angle;
    }
    else if (target_angle > missile->angle) {
        missile->angle += MISSILE_ROTATE_SPEED;
    }
    else {
        missile->angle -= MISSILE_ROTATE_SPEED;
    }
    //handle when angle wraps around (SBL only goes from -180 to 180)
    if (missile->angle > MTH_FIXED(180)) {
        missile->angle -= MTH_FIXED(360);
    }
    else if (missile->angle < MTH_FIXED(-180)) {
        missile->angle += MTH_FIXED(360);
    }
    missile->xPos += MTH_Mul(MTH_Cos(missile->angle), MISSILE_SPEED);
    missile->yPos += MTH_Mul(MTH_Sin(missile->angle), MISSILE_SPEED);

    //if middle of missile touches a wall, destroy the missile
    if (collision_check_point(missile->xPos + MTH_FIXED(8), missile->yPos + MTH_FIXED(8))) {
        sprite_delete(missile);
        explosion_make(missile->xPos - MTH_FIXED(8), missile->yPos - MTH_FIXED(8));
    }
    //if missile hits the player, kill him
    if (collision_player(missile)) {
        player_die();
    }
}

