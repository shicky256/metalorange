#ifndef SPRITE_H
#define SPRITE_H

#include <sega_def.h>
#include <sega_mth.h>

#define MIRROR_HORIZ (1 << 4)
#define MIRROR_VERT (1 << 5)

struct SpriteInfo;

typedef void (*IterateFunc)(struct SpriteInfo *);

//if the sprite shouldn't be displayed
#define OPTION_NODISP (1 << 0)
//if we're on a slope
#define OPTION_SLOPE (1 << 1)

#define SPRITE_GRAVITY (MTH_FIXED(0.5))
#define SPRITE_MAXFALLSPEED (MTH_FIXED(14))

typedef struct SpriteInfo {
	Uint16 char_num; //tile number
	Uint16 index; //where the sprite is in the sprites array
	Uint16 options;
	Uint16 state;
	Uint16 collision;
	Fixed32 x;
	Fixed32 y;
	Fixed32 x_size;
	Fixed32 y_size;
	Fixed32 dx;
	Fixed32 dy;
	Fixed32 scale;
	Fixed32 angle;
	Uint16 mirror;
	Uint16 anim_timer; //timer for animations
	Uint16 anim_cursor; //where we are in animation array
	IterateFunc iterate;
} SPRITE_INFO;

#define SPRITE_LIST_SIZE (160)
extern SPRITE_INFO sprites[];
//buffer in HWRAM to load sprite graphics in since you can't dma from LWRAM
#define SPRITE_BUF_SIZE (8192)
extern Uint8 sprite_buf[];

//sets up initial sprite display
void sprite_init(void);
//erases framebuffer (run at start of draw command)
void sprite_erase(Sint16 x, Sint16 y);
//gets vdp1 ready for draw commands
void sprite_startdraw(void);
//automatically picks the simplest SBL function for drawing the sprite depending
//on required features
//needs command to be opened before calling
void sprite_draw(SPRITE_INFO *info);
//inits the SPRITE_INFO pointer given
void sprite_make(int tile_num, Fixed32 x, Fixed32 y, SPRITE_INFO *ptr);
//draws all sprites in the sprite list
void sprite_draw_all(void);
//gets a pointer to the next sprite in the list
SPRITE_INFO *sprite_next(void);
//deletes the given sprite from the sprite list
void sprite_delete(SPRITE_INFO *sprite);
//deletes all sprites from the list
void sprite_deleteall(void);
//utility function that moves a sprite based on its state
//collision: 1 to do collision, 0 to not
//returns the tile the sprite is over
Uint16 sprite_move(SPRITE_INFO *sprite, int collision);

#endif
