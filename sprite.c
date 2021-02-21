#include <sega_def.h>
#include <sega_mth.h>
#include <machine.h>
#define _SPR2_
#include <sega_spr.h>
#include <sega_scl.h>
#include <string.h>

#include "cd.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

int num_sprites = 0;

int sprite_tilecnt = 0;
int sprite_palcnt = 0;
SPRITE_INFO sprites[SPRITE_LIST_SIZE];
//normalize diagonal speed
#define DIAGONAL_MULTIPLIER (MTH_FIXED(0.8))

#define CommandMax    300
#define GourTblMax    300
#define LookupTblMax  100
#define CharMax       256 //CHANGE WHEN YOU INCREASE TILES BEYOND THIS POINT
#define DrawPrtyMax   256
SPR_2DefineWork(work2D, CommandMax, GourTblMax, LookupTblMax, CharMax, DrawPrtyMax)
#define ENDCODE_DISABLE (1 << 7)

Uint8 sprite_buf[SPRITE_BUF_SIZE];


void sprite_init() {
	SCL_Vdp2Init();
	SCL_SetDisplayMode(SCL_DOUBLE_INTER, SCL_240LINE, SCL_HIRESO_B);
	SPR_2Initial(&work2D);
	SPR_2SetTvMode(SPR_TV_HIRESO, SPR_TV_704X240, ON);
	SCL_SetColRamMode(SCL_CRM24_1024);
	SCL_AllocColRam(SCL_SPR, 256, OFF);

	SetVblank(); //setup vblank routine
	set_imask(0);
	
	SPR_2FrameChgIntr(-1); //wait until next frame to set color mode
	SPR_2FrameEraseData(RGB16_COLOR(0, 0, 0)); //zero out frame
	SCL_DisplayFrame();

	// SCL_AllocColRam(SCL_SPR, 256, OFF);
	sprite_deleteall();
	SCL_DisplayFrame();
}

void sprite_erase(Sint16 x, Sint16 y) {
	XyInt xy[4];
	xy[0].x = 0; xy[0].y = 0; //upper left
	xy[1].x = x; xy[1].y = 0; //upper right
	xy[2].x = x; xy[2].y = y; //lower right
	xy[3].x = 0; xy[3].y = y; //lower right
	SPR_2Polygon(0, SPD_DISABLE, 0, xy, NO_GOUR);
}

void sprite_clear() {
	sprite_tilecnt = 0;
	sprite_palcnt = 0;
	SPR_2ClrAllChar();
}

int sprite_load(char *filename, int *count) {
	cd_load(filename, sprite_buf);
	Uint8 *sprite_bufptr = sprite_buf;
	Sint32 num_pals;
	memcpy(&num_pals, sprite_bufptr, sizeof(num_pals));
	sprite_bufptr += sizeof(num_pals);

	// load all the palettes
	for (int i = 0; i < num_pals; i++) {
		SCL_SetColRam(SCL_SPR, i + sprite_palcnt, 16, sprite_bufptr);
		sprite_bufptr += 16 * sizeof(Sint32); // move to next palette
	}

	// first 4 bytes after palettes is the number of sprites
	Sint32 num_sprites;
	memcpy(&num_sprites, sprite_bufptr, sizeof(num_sprites));
	sprite_bufptr += sizeof(num_sprites);
	// load all the sprites
	Sint32 sprite_x;
	Sint32 sprite_y;
	Sint32 sprite_pal;
	for (int i = 0; i < num_sprites; i++) {
		memcpy(&sprite_x, sprite_bufptr, sizeof(sprite_x));
		sprite_bufptr += sizeof(sprite_x);
		memcpy(&sprite_y, sprite_bufptr, sizeof(sprite_y));
		sprite_bufptr += sizeof(sprite_y);
		memcpy(&sprite_pal, sprite_bufptr, sizeof(sprite_pal));
		sprite_pal = (sprite_pal * 16) + sprite_palcnt;
		sprite_bufptr += sizeof(sprite_pal);
		SPR_2SetChar((Uint16)(i + sprite_tilecnt), COLOR_0, (Uint16)(sprite_pal),
		  (Uint16)sprite_x, (Uint16)sprite_y, sprite_bufptr);
		sprite_bufptr += ((sprite_x / 2) * sprite_y);
	}
	int sprite_tilebak = sprite_tilecnt;
	sprite_tilecnt += num_sprites;
	sprite_palcnt += (num_pals * 16);
	if (count) {
		*count = num_sprites;
	}
	return sprite_tilebak;
}

void sprite_startdraw(void) {
	XyInt xy;

	SPR_2OpenCommand(SPR_2DRAW_PRTY_OFF);
	if (scroll_res == SCROLL_RES_HIGH) {
		xy.x = 704;
		xy.y = 480;
	}
	else {
		xy.x = 352;
		xy.y = 240;
	}
	SPR_2SysClip(0, &xy);
	sprite_erase(xy.x, xy.y);
}

void sprite_draw(SPRITE_INFO *info) {
	XyInt xy[4];
	Fixed32 xOffset, yOffset, sin, cos, scaledX, scaledY;

	if (info->scale == MTH_FIXED(1) && info->angle == 0) {
		xy[0].x = (Sint16)MTH_FixedToInt(info->x);
		xy[0].y = (Sint16)MTH_FixedToInt(info->y);
		SPR_2NormSpr(0, info->mirror, ENDCODE_DISABLE | (1 << 11) | (1 << 12), 0xffff, info->char_num, xy, NO_GOUR); //4bpp normal sprite
	}
	
	else if (info->angle == 0){	
		xy[0].x = (Sint16)MTH_FixedToInt(info->x);
		xy[0].y = (Sint16)MTH_FixedToInt(info->y);
		//the way scale works is by giving the x/y coordinates of the top left and
		//bottom right corner of the sprite
		xy[1].x = (Sint16)(MTH_FixedToInt(MTH_Mul(info->x_size, info->scale) + info->x));
		xy[1].y = (Sint16)(MTH_FixedToInt(MTH_Mul(info->y_size, info->scale) + info->y));
		SPR_2ScaleSpr(0, info->mirror, ENDCODE_DISABLE, 0xffff, info->char_num, xy, NO_GOUR); //4bpp scaled sprite
	}
	
	else {
		//offset of top left sprite corner from the origin
		xOffset = -(MTH_Mul(info->x_size >> 1, info->scale));
		yOffset = -(MTH_Mul(info->y_size >> 1, info->scale));
		sin = MTH_Sin(info->angle);
		cos = MTH_Cos(info->angle);
		scaledX = info->x + MTH_Mul(info->x_size >> 1, info->scale);
		scaledY = info->y + MTH_Mul(info->y_size >> 1, info->scale);
		//formula from
		//https://gamedev.stackexchange.com/questions/86755/
		for (int i = 0; i < 4; i++) {
			if (i == 1) xOffset = -xOffset; //upper right
			if (i == 2) yOffset = -yOffset; //lower right
			if (i == 3) xOffset = -xOffset; //lower left
			xy[i].x = (Sint16)MTH_FixedToInt(MTH_Mul(xOffset, cos) - 
				MTH_Mul(yOffset, sin) + scaledX);
			xy[i].y = (Sint16)MTH_FixedToInt(MTH_Mul(xOffset, sin) +
				MTH_Mul(yOffset, cos) + scaledY);
		}
		SPR_2DistSpr(0, info->mirror, ENDCODE_DISABLE, 0xffff, info->char_num, xy, NO_GOUR); //4bpp distorted sprite
	}
}

void sprite_make(int tile_num, Fixed32 x, Fixed32 y, SPRITE_INFO *ptr) {
	ptr->display = 1;
	ptr->char_num = tile_num;
	ptr->x = x;
	ptr->y = y;
	ptr->x_size = 0;
	ptr->y_size = 0;
	ptr->mirror = 0;
	ptr->scale = MTH_FIXED(1);
	ptr->angle = 0;
	ptr->prev = NULL;
	ptr->next = NULL;
	ptr->iterate = NULL;
}

void sprite_draw_all() {
	for (int i = 0; i < SPRITE_LIST_SIZE; i++) {
		if (sprites[i].display) {
			if (sprites[i].iterate != NULL) {
				sprites[i].iterate(&sprites[i]);
			}
			//check again because iterate function may have deleted sprite
			if (sprites[i].display) {
				sprite_draw(&sprites[i]);
			}
		}
	}
}

SPRITE_INFO *sprite_next() {
	int i;
	for (i = 0; i < SPRITE_LIST_SIZE; i++) {
		if (!sprites[i].display) {
			num_sprites++;
			sprites[i].index = i;
			sprites[i].iterate = NULL;
			return &sprites[i];
		}
	}
	return NULL;
}

void sprite_listadd(SPRITE_INFO **head, SPRITE_INFO *sprite) {
	sprite->next = *head;
	sprite->prev = NULL;
	if (*head != NULL) {
		(*head)->prev = sprite;
	}
	*head = sprite;
}

void sprite_listremove(SPRITE_INFO **head, SPRITE_INFO *sprite) {
	if (*head == sprite) {
		*head = sprite->next;
	}

	if (sprite->next != NULL) {
		sprite->next->prev = sprite->prev;
	}

	if (sprite->prev != NULL) {
		sprite->prev->next = sprite->next;
	}
}

void sprite_delete(SPRITE_INFO *sprite) {
	sprite->display = 0;
	sprite->iterate = NULL;
	num_sprites--;
}

void sprite_deleteall() {
	for (int i = 0; i < SPRITE_LIST_SIZE; i++) {
		sprites[i].display = 0;
	}
	num_sprites = 0;
}

