#include <string.h> //memcpy
#define	_SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>

#include "cd.h"
#include "devcart.h"
#include "graphicrefs.h"
#include "intro.h"
#include "logo.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"
#include "vblank.h"

Uint32 frame = 0;
SclConfig off_config;

typedef enum {
	STATE_LOGO = 0,
	STATE_INTRO
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

	off_config.dispenbl = OFF;

	devcart_printstr("hello, i am a sega saturn\r\n");
	int state = STATE_LOGO;
	while(1) {
		sprite_startdraw();
		
		switch (state) {
			case STATE_LOGO:
				if (logo_run() || PadData1 & PAD_S) {
					state = STATE_INTRO;
				}
				break;
			
			case STATE_INTRO:
				intro_run();
				break;
		}

		frame++;
		print_num(frame, 5, 5);
		//if the cd drive is opened, return to menu
		CDC_GetPeriStat(&cd_status);
		if ((cd_status.status & 0xF) == CDC_ST_OPEN) {
			SYS_EXECDMP();
		}
		//if player hits A+B+C+Start, return to menu
		if (PadData1 == (PAD_A | PAD_B | PAD_C | PAD_S)) {
			SYS_EXECDMP();
		}
			// sprite_draw_all();
		print_display();
		SPR_2CloseCommand();

		SCL_DisplayFrame();
	}
	return 0;
}
