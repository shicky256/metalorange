#include <string.h>
#include <sega_def.h>
#include <sega_mth.h>
#include <sega_scl.h>
#define	_SPR2_
#include <sega_spr.h>

#include "cd.h"
#include "graphicrefs.h"
#include "print.h"
#include "scroll.h"
#include "sprite.h"

Uint32 vram[] = {SCL_VDP2_VRAM_A0, SCL_VDP2_VRAM_A0 + 0x10000, SCL_VDP2_VRAM_B0, SCL_VDP2_VRAM_B0 + 0x10000}; //where in VRAM each tilemap is
int scroll_res = SCROLL_RES_LOW;


Uint32 char_offsets[] = {
	SCL_VDP2_VRAM_A1 - SCL_VDP2_VRAM, //NBG0
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG1
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG2
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG3
};

/*
 * 0: NBG0 Pattern Name
 * 1: NBG1 Pattern Name
 * 2: NBG2 Pattern Name
 * 3: NBG3 Pattern Name
 * 4: NBG0 Character Pattern
 * 5: NBG1 Character Pattern
 * 6: NBG2 Character Pattern
 * 7: NBG3 Character Pattern
 * C: NBG0 Vertical Scroll Table
 * D: NBG1 Vertical Scroll Table
 * E: CPU Read/Write
 * F: No Access
 */

/*
 Data Type			# Accesses required
 Pattern name data          1
 16-color tiles		  		1
 256-color tiles	  		2
 2048-color tiles	  		4
 32K-color tiles	  		4
 16M-color tiles	  		8
 Vertical scroll data 		1
 */

// There's also numerous read restrictions, see SOA technical bulletin #6 for more information
//lo res vram layout: 
//nbg 0/1 tilemaps in A0
//nbg 0 graphics in A1
//nbg 2/3 graphics in B0
//nbg 2/3 tilemaps in B1
//nbg 0 is 256 color, 1, 2, & 3 are 16 color
Uint16	CycleTbLoRes[]={
	0x01ee,0xeeee,
	0x44ee,0xeeee,
	0xff23,0xffff,
	0x756f,0xeeee
};

//in hi-res mode, the last 4 timing cycles aren't valid
//hi res vram layout:
//nbg 0/1 tilemaps in A0
//nbg 0 graphics in A1
//nbg 2/3 tilemaps in B0
//nbg 1/2/3 graphics in B1
//nbg 0 is 256 color, 1, 2 & 3 are 16 color
Uint16	CycleTbHiRes[]={
	0x01ee,0xffff,
	0x44ee,0xffff,
	0xff23,0xffff,
	0x756f,0xffff
};

SclConfig scfg[4];

void scroll_init(void) {
	int i;
	Uint16 BackCol;
	SclVramConfig vram_cfg;

	//wipe out vram
	memset((void *)SCL_VDP2_VRAM, 0, 0x80000);

	SCL_SetColRamMode(SCL_CRM24_1024);
	SCL_AllocColRam(SCL_NBG0, 256, OFF);
	SCL_AllocColRam(SCL_NBG1 | SCL_NBG2 | SCL_NBG3, 64, OFF);
		// SCL_AllocColRam(SCL_NBG2, 256, OFF);
		// SCL_SetColRam(SCL_NBG2, 0, 256, (void *)(level->playfield.palette));

		// SCL_AllocColRam(SCL_NBG0, 16, OFF);
		// SCL_SetColRam(SCL_NBG0, 0, 16, (void *)(level->bg_far.palette));		

		// SCL_AllocColRam(SCL_NBG1, 16, OFF);
		// SCL_SetColRam(SCL_NBG1, 0, 16, (void *)(level->bg_near.palette));

	BackCol = 0x0000; //set the background color to black
	SCL_SetBack(SCL_VDP2_VRAM+0x80000-2,1,&BackCol);

	SCL_InitConfigTb(&scfg[0]);
		scfg[0].dispenbl      = ON;
		scfg[0].charsize      = SCL_CHAR_SIZE_2X2;
		scfg[0].pnamesize     = SCL_PN1WORD;
		scfg[0].flip          = SCL_PN_10BIT;
		scfg[0].platesize     = SCL_PL_SIZE_2X1; //they meant "plane size"
		scfg[0].coltype       = SCL_COL_TYPE_256;
		scfg[0].datatype      = SCL_CELL;
		scfg[0].patnamecontrl = 0x0004; //vram A1 offset
		scfg[0].plate_addr[0] = vram[0];
		scfg[0].plate_addr[1] = vram[0] + 0x800;
	SCL_SetConfig(SCL_NBG0, &scfg[0]);

	memcpy((void *)&scfg[1], (void *)&scfg[0], sizeof(SclConfig));
	scfg[1].dispenbl = ON;
	scfg[1].platesize = SCL_PL_SIZE_1X1;
	scfg[1].coltype = SCL_COL_TYPE_16;
	scfg[1].patnamecontrl = 0x000c;
	for(i=0;i<4;i++)   scfg[1].plate_addr[i] = vram[1];
	SCL_SetConfig(SCL_NBG1, &scfg[1]);

	memcpy((void *)&scfg[2], (void *)&scfg[1], sizeof(SclConfig));
	scfg[2].dispenbl = ON;
	for(i=0;i<4;i++)   scfg[2].plate_addr[i] = vram[2];
	SCL_SetConfig(SCL_NBG2, &scfg[2]);

	memcpy((void *)&scfg[3], (void *)&scfg[2], sizeof(SclConfig));
	scfg[3].dispenbl = ON;
	for(i=0;i<4;i++)   scfg[3].plate_addr[i] = vram[3];
	SCL_SetConfig(SCL_NBG3, &scfg[3]);
	
	//setup VRAM configuration
	SCL_InitVramConfigTb(&vram_cfg);
		vram_cfg.vramModeA = ON; //separate VRAM A into A0 & A1
		vram_cfg.vramModeB = ON; //separate VRAM B into B0 & B1
	SCL_SetVramConfig(&vram_cfg);

	//setup vram access pattern
	SCL_SetCycleTable(CycleTbLoRes);
	 
	SCL_Open(SCL_NBG0);
		SCL_MoveTo(FIXED(0), FIXED(0), 0); //home position
	SCL_Close();
	SCL_Open(SCL_NBG1);
		SCL_MoveTo(FIXED(0), FIXED(0), 0);
	SCL_Close();
	SCL_Open(SCL_NBG2);
		SCL_MoveTo(FIXED(0), FIXED(0), 0);
	SCL_Close();
	SCL_Open(SCL_NBG3);
		SCL_MoveTo(FIXED(0), FIXED(0), 0);
	SCL_Close();
	scroll_scale(0, FIXED(1));
	scroll_scale(1, FIXED(1));
	SCL_SetPriority(SCL_SPR,  7);
	SCL_SetPriority(SCL_NBG0, 6);
	SCL_SetPriority(SCL_NBG1, 5);
	SCL_SetPriority(SCL_NBG2, 4);
	SCL_SetPriority(SCL_NBG3, 3); //set layer priorities
	
}

void scroll_lores() {
	SCL_DisplayFrame();
	SCL_DisplayFrame();
	SCL_SetDisplayMode(SCL_NON_INTER, SCL_240LINE, SCL_NORMAL_B);
	SPR_2SetTvMode(SPR_TV_NORMAL, SPR_TV_352X240, OFF);
	SCL_SetCycleTable(CycleTbLoRes);
	scroll_res = SCROLL_RES_LOW;
}

void scroll_hires() {
	SCL_DisplayFrame();
	SCL_DisplayFrame();
	SCL_SetDisplayMode(SCL_DOUBLE_INTER, SCL_240LINE, SCL_HIRESO_B);
	SPR_2SetTvMode(SPR_TV_HIRESO, SPR_TV_704X240, ON);
	SCL_SetCycleTable(CycleTbHiRes);
	scroll_res = SCROLL_RES_HIGH;
}

#define ZOOM_HALF_NBG0 (0x1)
#define ZOOM_QUARTER_NBG0 (0x2)
#define ZOOM_HALF_NBG1 (0x100)
#define ZOOM_QUARTER_NBG1 (0x200)
#define LOW_BYTE (0xFF)
#define HIGH_BYTE (0xFF00)

void scroll_scale(int num, Fixed32 scale) {
	SCL_Open(1 << (num + 2));
		SCL_Scale(scale, scale);
	SCL_Close();
	//reset the configuration byte for the given background
	Scl_n_reg.zoomenbl &= (num == 0 ? HIGH_BYTE : LOW_BYTE);
	if (scale >= FIXED(1)) {
		Scl_n_reg.zoomenbl &= (num == 0 ? ~(ZOOM_HALF_NBG0 | ZOOM_QUARTER_NBG0)
										: ~(ZOOM_HALF_NBG1 | ZOOM_QUARTER_NBG1));
		return;
	}
	else if (scale < FIXED(1) && scale >= FIXED(0.5)) {
		Scl_n_reg.zoomenbl |= (num == 0 ? ZOOM_HALF_NBG0 : ZOOM_HALF_NBG1);
	}
	else {
		Scl_n_reg.zoomenbl |= (num == 0 ? ZOOM_QUARTER_NBG0 : ZOOM_QUARTER_NBG1);
	}
}

void scroll_set(int num, Fixed32 x, Fixed32 y) {
	SCL_Open(1 << (num + 2));
	SCL_MoveTo(x, y, 0);
	SCL_Close();
}

void scroll_move(int num, Fixed32 x, Fixed32 y) {
	SCL_Open(1 << (num + 2));
	SCL_Move(x, y, 0);
	SCL_Close();
}

void scroll_clearmaps(void) {
	memset(MAP_PTR(0), 0, 0x2000);
	memset(MAP_PTR(1), 0, 0x2000);
	memset(MAP_PTR(2), 0, 0x2000);
	memset(MAP_PTR(3), 0, 0x2000);
}

void scroll_charsize(int num, Uint8 size) {
	scfg[num].charsize = size;
	SCL_SetConfig(1 << (num + 2), &scfg[num]);
}

void scroll_mapsize(int num, Uint8 size) {
	scfg[num].pnamesize = size;
	SCL_SetConfig(1 << (num + 2), &scfg[num]);
}
