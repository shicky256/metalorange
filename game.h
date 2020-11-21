#ifndef GAME_H
#define GAME_H
#include "sprite.h"

#define LEFT_WALL (MTH_FIXED(16))
#define RIGHT_WALL (MTH_FIXED(224))
//leftmost x pos the ship can go
#define LEFT_BOUND (LEFT_WALL - SHIP_XMARGIN)
//rightmost x pos the ship can go
#define RIGHT_BOUND (RIGHT_WALL - MTH_FIXED(ship_width) + SHIP_XMARGIN)

//the area between where the sprite is and where the actual pixel graphics start
#define SHIP_XMARGIN (MTH_FIXED(4))
#define SHIP_YMARGIN (MTH_FIXED(8))
#define SHIP_WIDTH (MTH_FIXED(40))
#define SHIP_HEIGHT (MTH_FIXED(24))
#define SHIP_STATE_INIT (0)
#define SHIP_STATE_NORM (1)

extern SPRITE_INFO *ship_sprite;

int game_run();
//gives the player a powerup
void game_incpowerup();
//running this kills the player (in the game, not real life)
void game_loss();

#endif
