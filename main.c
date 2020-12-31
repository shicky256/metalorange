#include <string.h> //memcpy
#define	_SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>
#include <SEGA_PER.H>

#include "cd.h"
#include "cutscene.h"
#include "devcart.h"
#include "game.h"
#include "graphicrefs.h"
#include "intro.h"
#include "logo.h"
#include "menu.h"
#include "notice.h"
#include "release.h"
#include "scroll.h"
#include "soon.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"
#include "vblank.h"

Uint32 frame = 0;
SclConfig off_config;

typedef enum {
	STATE_NOTICE = 0,
	STATE_LOGO,
	STATE_INTRO,
	STATE_MENU,
	STATE_CUTSCENE,
	STATE_GAME,
	STATE_SOON,
} GAME_STATE;

int main() {
	CdcStat cd_status;

	cd_init();
	sprite_init();
	scroll_init();
	print_init();
	print_load();
	sound_init();
	//init RNG
	Uint8 *time = PER_GET_TIM();
	Uint32 seed = time[4] | (time[3] << 8) | (time[2] << 16) | (time[3] << 24);
	MTH_InitialRand(seed);

	off_config.dispenbl = OFF;

	int state = STATE_NOTICE;
	while(1) {
		sprite_startdraw();
		
		switch (state) {
			case STATE_NOTICE:
				if (notice_run()) {
					state = STATE_LOGO;
				}
				break;

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
					if (cutscene) {
						state = STATE_CUTSCENE;
					}
					else {
						state = STATE_GAME;
					}
				}
				break;

			case STATE_CUTSCENE:
				if (cutscene_run()) {
					state = STATE_GAME;
				}
				break;

			case STATE_GAME:;
				int result = game_run();
				if (result == 1) { // completed
					state = STATE_SOON;
				}
				else if (result == 2) { // game over
					state = STATE_LOGO;
				}
				break;

			case STATE_SOON:
				if (soon_run()) {
					state = STATE_LOGO;
				}
				break;
		}

		frame++;
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
		#if DEBUG
		print_display();
		#endif
		SPR_2CloseCommand();

		SCL_DisplayFrame();
	}
	return 0;
}
