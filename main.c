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

int main() {
	CdcStat cd_status;

	cd_init();
	sprite_init();
	scroll_init();
	scroll_hires();
	print_init();
	SCL_SetSpriteMode(SCL_TYPE8, SCL_PALETTE, SCL_SP_WINDOW);
	title_init();
	sound_init();
	sound_cdda(2);

	off_config.dispenbl = OFF;


	while(1) {
		frame++;

		title_run();
		trigger_t PadData1EW = PadData1E;
		if (PadData1 == PAD_S) {
			SYS_EXECDMP();
		}

		//704x480i
		if (PadData1EW & PAD_A) {
			scroll_hires();
		}
		//352x240p
		else if (PadData1EW & PAD_B) {
			scroll_lores();
		}

		print_num(frame, 5, 5);
		//if the cd drive is opened, return to menu
		CDC_GetPeriStat(&cd_status);
		if ((cd_status.status & 0xF) == CDC_ST_OPEN) {
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
