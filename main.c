#include <string.h> //memcpy
#define	_SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>

#include "cd.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"
#include "title.h"
#include "vblank.h"

Uint32 frame = 0;
SclConfig off_config;

typedef enum {
	STATE_TITLE_INIT = 0,
	STATE_TITLE_FADEIN,
	STATE_TITLE_RUN,
	STATE_TITLE_DONE,
	STATE_TITLE_FADEOUT,
	STATE_TITLE_END
} GAME_STATE;

int main() {
	CdcStat cd_status;
	SclRgb start, end;

	cd_init();
	sprite_init();
	scroll_init();
	scroll_hires();
	print_init();
	SCL_SetSpriteMode(SCL_TYPE8, SCL_PALETTE, SCL_SP_WINDOW);
	sound_init();

	off_config.dispenbl = OFF;

	int state = STATE_TITLE_INIT;
	while(1) {
		switch(state) {
			case STATE_TITLE_INIT:
				//fade out title screen
				SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255);
				SCL_DisplayFrame();
				title_init();
				start.red = start.green = start.blue = -255;
				end.red = end.green = end.blue = 0;
				SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 60, &start, &end);
				frame = 0;
				state = STATE_TITLE_FADEIN;
				break;
			
			case STATE_TITLE_FADEIN:
				if (frame > 60) {
					sound_cdda(2); //play logo song
					state = STATE_TITLE_RUN;
				}
				break;

			case STATE_TITLE_RUN:
				if (title_run()) {
					frame = 0;
					state = STATE_TITLE_DONE;					
				}
				break;

			case STATE_TITLE_DONE:
				if (frame > 60) {
					SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 60, &end, &start);
					frame = 0;
					state = STATE_TITLE_FADEOUT;
				}
				break;
				
			case STATE_TITLE_FADEOUT:
				if (frame > 60) {
					state = STATE_TITLE_END;
				}
				break;

			case STATE_TITLE_END:
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
		sprite_startdraw();
			// sprite_draw_all();
			print_display();
		SPR_2CloseCommand();

		SCL_DisplayFrame();
	}
	return 0;
}
