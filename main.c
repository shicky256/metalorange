#include <string.h> //memcpy
#define	_SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>
#include <SEGA_PER.H>

#include "cd.h"
#include "devcart.h"
#include "game.h"
#include "graphicrefs.h"
#include "intro.h"
#include "logo.h"
#include "menu.h"
#include "release.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"
#include "vblank.h"

Uint32 frame = 0;
SclConfig off_config;

typedef enum {
	STATE_LOGO = 0,
	STATE_INTRO,
	STATE_MENU,
	STATE_GAME,
} GAME_STATE;

int main() {
	CdcStat cd_status;

	cd_init();
	sprite_init();
	scroll_init();
	scroll_hires();
	print_init();
	SCL_SetSpriteMode(SCL_TYPE8, SCL_PALETTE, SCL_SP_WINDOW);
	sound_init();
	//init RNG
	Uint8 *time = PER_GET_TIM();
	Uint32 seed = time[4] | (time[3] << 8) | (time[2] << 16) | (time[3] << 24);
	MTH_InitialRand(seed);

	off_config.dispenbl = OFF;

	int state = STATE_GAME;
	while(1) {
		sprite_startdraw();
		
		switch (state) {
			case STATE_LOGO:
				if (logo_run()) {
					state = STATE_INTRO;
				}
				break;
			
			case STATE_INTRO:
				if (intro_run()) {
					state = STATE_MENU;
				}
				break;
			
			case STATE_MENU:
				if (menu_run()) {
					state = STATE_GAME;
				}
				break;

			case STATE_GAME:
				game_run();
				break;
		}

		frame++;
		print_num(frame, 5, 5);
		//if the cd drive is opened, return to menu
		CDC_GetPeriStat(&cd_status);
		if ((cd_status.status & 0xF) == CDC_ST_OPEN) {
			#if DEVCART_LOAD
			#else
			SYS_EXECDMP();
			#endif
		}
		//if player hits A+B+C+Start, return to menu
		if (PadData1 == (PAD_A | PAD_B | PAD_C | PAD_S)) {
			#if DEVCART_LOAD
			devcart_reset();
			#else
			SYS_EXECDMP();
			#endif
		}
		sprite_draw_all();
		print_display();
		SPR_2CloseCommand();

		SCL_DisplayFrame();
	}
	return 0;
}
