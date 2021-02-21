#ifndef SPRITE_H
#define SPRITE_H

#include <sega_def.h>
#include <sega_mth.h>

#define MIRROR_HORIZ (1 << 4)
#define MIRROR_VERT (1 << 5)

struct SpriteInfo;
typedef struct SpriteInfo SPRITE_INFO;

typedef void (*IterateFunc)(SPRITE_INFO *);

#define SPRITE_DATA_SIZE (12)

typedef struct SpriteInfo {
	Uint16 display;
	Uint16 char_num; //tile number
	Uint16 index; //where the sprite is in the sprites array
	Fixed32 x;
	Fixed32 y;
	Fixed32 x_size;
	Fixed32 y_size;
	Fixed32 scale;
	Fixed32 angle;
	Uint16 mirror;
	SPRITE_INFO *prev; // for iterating through a certain type of sprite
	SPRITE_INFO *next;
	Uint8 data[SPRITE_DATA_SIZE] __attribute__((aligned(4)));
	IterateFunc iterate;
} SPRITE_INFO;

#define SPRITE_LIST_SIZE (80)
extern SPRITE_INFO sprites[];
//buffer in HWRAM to load sprite graphics in since you can't dma from LWRAM
#define SPRITE_BUF_SIZE (65536)
extern Uint8 sprite_buf[];

//sets up initial sprite display
void sprite_init(void);
//erases framebuffer (run at start of draw command)
void sprite_erase(Sint16 x, Sint16 y);
// clears vdp1 memory
void sprite_clear(void);
// loads a sprite off the disc, returns the tile number of the first loaded sprite
// count: optional parameter, if it's not null gets set to # of sprites loaded
int sprite_load(char *filename, int *count);
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
// adds the sprite to a doubly linked list
void sprite_listadd(SPRITE_INFO **head, SPRITE_INFO *sprite);
// removes the sprite from a doubly linked list
void sprite_listremove(SPRITE_INFO **head, SPRITE_INFO *sprite);
//deletes the given sprite from the sprite list
void sprite_delete(SPRITE_INFO *sprite);
//deletes all sprites from the list
void sprite_deleteall(void);
//utility function that moves a sprite based on its state
//collision: 1 to do collision, 0 to not
//returns the tile the sprite is over
Uint16 sprite_move(SPRITE_INFO *sprite, int collision);

#endif
