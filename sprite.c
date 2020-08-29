#include <sega_def.h>
#include <sega_mth.h>
#include <machine.h>
#define _SPR2_
#include <sega_spr.h>
#include <sega_scl.h>
#include <string.h>

#include "cd.h"
#include "graphicrefs.h"
#include "print.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

int num_sprites = 0;
SPRITE_INFO sprites[SPRITE_LIST_SIZE];
//normalize diagonal speed
#define DIAGONAL_MULTIPLIER (MTH_FIXED(0.8))

#define CommandMax    300
#define GourTblMax    300
#define LookupTblMax  100
#define CharMax       128 //CHANGE WHEN YOU INCREASE TILES BEYOND THIS POINT
#define DrawPrtyMax   256
SPR_2DefineWork(work2D, CommandMax, GourTblMax, LookupTblMax, CharMax, DrawPrtyMax)

Uint8 sprite_buf[SPRITE_BUF_SIZE];


void sprite_init() {
	int count, i, char_base;

	SCL_Vdp2Init();
	SCL_SetDisplayMode(SCL_DOUBLE_INTER, SCL_240LINE, SCL_HIRESO_B);
	SPR_2Initial(&work2D);
	SPR_2SetTvMode(SPR_TV_HIRESO, SPR_TV_704X240, ON);
	count = 0;
	SCL_SetColRamMode(SCL_CRM24_1024);
	SCL_AllocColRam(SCL_SPR, 256, OFF);

	SetVblank(); //setup vblank routine
	set_imask(0);
	
	SPR_2FrameChgIntr(-1); //wait until next frame to set color mode
	SPR_2FrameEraseData(RGB16_COLOR(0, 0, 0)); //zero out frame
	SCL_DisplayFrame();
	
	cd_load(font_name, sprite_buf, font_size * font_num);
	for (i = 0; i < font_num; i++) {
		SPR_2SetChar(i, COLOR_0, 0, font_width, font_height, sprite_buf + (i * font_size));
	}
	char_base = font_num;
	SCL_AllocColRam(SCL_SPR, 96, OFF);
	SCL_SetColRam(SCL_SPR, 0, 16, &font_pal);
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
		SPR_2NormSpr(0, info->mirror, 0, 0xffff, info->char_num, xy, NO_GOUR); //4bpp normal sprite
	}
	
	else if (info->angle == 0){
		xy[0].x = (Sint16)MTH_FixedToInt(info->x);
		xy[0].y = (Sint16)MTH_FixedToInt(info->y);
		//the way scale works is by giving the x/y coordinates of the top left and
		//bottom right corner of the sprite
		xy[1].x = (Sint16)(MTH_FixedToInt(MTH_Mul(info->x_size, info->scale) + info->x));
		xy[1].y = (Sint16)(MTH_FixedToInt(MTH_Mul(info->y_size, info->scale) + info->y));
		SPR_2ScaleSpr(0, info->mirror, 0, 0xffff, info->char_num, xy, NO_GOUR); //4bpp scaled sprite
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
		SPR_2DistSpr(0, info->mirror, 0, 0xffff, info->char_num, xy, NO_GOUR); //4bpp distorted sprite
	}
}

void sprite_make(int tile_num, Fixed32 x, Fixed32 y, SPRITE_INFO *ptr) {
	ptr->char_num = tile_num;
	ptr->options = 0;
	ptr->state = 0;
	ptr->x = x;
	ptr->y = y;
	ptr->x_size = 0;
	ptr->y_size = 0;
	ptr->mirror = 0;
	ptr->dx = 0;
	ptr->dy = 0;
	ptr->scale = MTH_FIXED(1);
	ptr->angle = 0;
	ptr->anim_timer = 0;
	ptr->anim_cursor = 0;
	ptr->iterate = NULL;
}

void sprite_draw_all() {
	int i;
	Sint32 rel_x, rel_y;
	SPRITE_INFO tmp;
	print_num(num_sprites, 0, 0);
	for (i = 0; i < SPRITE_LIST_SIZE; i++) {
		rel_x = sprites[i].x;
		rel_y = sprites[i].y;
		//if sprite is more than 1/2 screen offscreen, don't render it
		if (sprites[i].options & OPTION_NODISP || rel_x + sprites[i].x_size < MTH_FIXED(-160) || rel_x > MTH_FIXED(480) ||
			 rel_y + sprites[i].y_size < MTH_FIXED(-112) || rel_y > MTH_FIXED(336)) {
			continue;
		}

		if (sprites[i].iterate != NULL) {
			sprites[i].iterate(&sprites[i]);
		}
		//check again because iterate function may have deleted sprite
		if (!(sprites[i].options & OPTION_NODISP)) {
			memcpy((void *)&tmp, (void *)&sprites[i], sizeof(SPRITE_INFO));
			tmp.x = rel_x;
			tmp.y = rel_y;
			sprite_draw(&tmp);
		}
	}
}

SPRITE_INFO *sprite_next() {
	int i;
	for (i = 0; i < SPRITE_LIST_SIZE; i++) {
		if (sprites[i].options & OPTION_NODISP) {
			num_sprites++;
			sprites[i].index = i;
			sprites[i].iterate = NULL;
			return &sprites[i];
		}
	}
	return NULL;
}

void sprite_delete(SPRITE_INFO *sprite) {
	sprite->options |= OPTION_NODISP;
	sprite->iterate = NULL;
	num_sprites--;
}

void sprite_deleteall() {
	for (int i = 0; i < SPRITE_LIST_SIZE; i++) {
		sprites[i].options = OPTION_NODISP;
	}
	num_sprites = 0;
}

