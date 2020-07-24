#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>
#define _SPR2_
#include <SEGA_SPR.H>

#include "cd.h"
#include "graphicrefs.h"
#include "menu.h"
#include "scroll.h"
#include "sprite.h"

#define CHIP_FRAME_SIZE (42 * 256)

static int chip_cursor;
#define CHIP_ANIMIN_NUMFRAMES (5)
static int chip_animin_timings[CHIP_ANIMIN_NUMFRAMES] = {50, 32, 16, 10, 10};
static char *chip_animin_frames[CHIP_ANIMIN_NUMFRAMES] = {(char *)LWRAM,
                                                          (char *)LWRAM + 1 * CHIP_FRAME_SIZE,
                                                          (char *)LWRAM,
                                                          (char *)LWRAM + 2 * CHIP_FRAME_SIZE,
                                                          (char *)LWRAM + 3 * CHIP_FRAME_SIZE};
#define CHIP_RUN_NUMFRAMES (4)
#define CHIP_RUN_TIMING (13)
static char *chip_run_frames[CHIP_RUN_NUMFRAMES] = {(char *)LWRAM + 4 * CHIP_FRAME_SIZE,
                                                    (char *)LWRAM + 5 * CHIP_FRAME_SIZE,
                                                    (char *)LWRAM + 6 * CHIP_FRAME_SIZE,
                                                    (char *)LWRAM + 7 * CHIP_FRAME_SIZE};

static char *tile_ptr = ((char *)SCL_VDP2_VRAM_A1) + 256;
static int timer;

#define STAR_CHARNO (font_num)

typedef enum {
    STATE_MENU_INIT,
    STATE_MENU_ANIMIN,
    STATE_MENU_RUN,
} MENU_STATE;

static int state = STATE_MENU_INIT;

static inline void menu_init() {
    char *menu_buf = (char *)LWRAM;
    Uint16 *map_ptr = VRAM_PTR(0);

    scroll_lores();
    //put NBG0 in a good location
    scroll_set(0, MTH_FIXED(-220), MTH_FIXED(-140));
    //load star gfx
    cd_load_nosize(starspr_name, sprite_buf);
    SCL_SetColRam(SCL_SPR, 16, 16, starspr_pal);
	for (int i = 0; i < starspr_num; i++) {
		SPR_2SetChar(i + STAR_CHARNO, COLOR_0, 16, starspr_width, starspr_height, sprite_buf + (i * starspr_size));
	}
    SCL_SetPriority(SCL_SPR, 2); //put spr scroll layer under everything
    cd_load_nosize(chipframes_name, menu_buf);
    DMA_CpuMemCopy1(tile_ptr, menu_buf, 256 * 42);
    int counter = 2;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            if (j < 7 && i < 6) {
                map_ptr[i * 32 + j] = counter;
                counter += 2;
            }
            else {
                map_ptr[i * 32 + j] = 0;
            }
        }
    }
    SCL_SetColRam(SCL_NBG0, 0, 256, chipframes_pal);
}

static void menu_starmove(SPRITE_INFO *star) {
    if ((star->x < 0) || (star->x > MTH_FIXED(SCROLL_LORES_X) ||
        (star->y < 0) || (star->y > MTH_FIXED(SCROLL_LORES_Y)))) {
        sprite_delete(star);
    }
    //slight acceleration
    MTH_Mul(star->dx, MTH_FIXED(1.5));
    MTH_Mul(star->dy, MTH_FIXED(1.5));
    star->x += star->dx;
    star->y += star->dy;
    //calculate distance using pythagorean theorem, use to calculate sprite scale
    Fixed32 distance_x = star->x - MTH_FIXED(SCROLL_LORES_X >> 1);
    Fixed32 distance_y = star->y - MTH_FIXED(SCROLL_LORES_Y >> 1);
    distance_x = MTH_Mul(distance_x, distance_x);
    distance_y = MTH_Mul(distance_y, distance_y);
    Fixed32 distance = MTH_Sqrt(distance_x + distance_y);
    star->scale = MTH_Div(distance, MTH_FIXED(80));

}

static inline void menu_starcreate(SPRITE_INFO *star) {
    Fixed32 x = (Fixed32)MTH_GetRand() % MTH_FIXED(5) + MTH_FIXED(SCROLL_LORES_X >> 1);
    Fixed32 y = (Fixed32)MTH_GetRand() % MTH_FIXED(5) + MTH_FIXED(SCROLL_LORES_Y >> 1);
    int tile_num = STAR_CHARNO + ((MTH_GetRand() >> 5) % starspr_num);
    sprite_make(tile_num, x, y, star);
    star->x_size = MTH_FIXED(starspr_width);
    star->y_size = MTH_FIXED(starspr_height);
    Fixed32 angle = (Fixed32)(MTH_GetRand() % MTH_FIXED(360));
    angle -= MTH_FIXED(180);
    star->dx = MTH_Mul(MTH_FIXED(5), MTH_Cos(angle));
    star->dy = MTH_Mul(MTH_FIXED(5), MTH_Sin(angle));
    star->iterate = menu_starmove;
}

int menu_run() {
    switch (state) {
        case STATE_MENU_INIT:
            menu_init();
            timer = 0;
            chip_cursor = 0;
            state = STATE_MENU_ANIMIN;
            break;

        case STATE_MENU_ANIMIN:
            timer++;
            if (timer == chip_animin_timings[chip_cursor]) {
                timer = 0;
                chip_cursor++;
                if (chip_cursor == CHIP_ANIMIN_NUMFRAMES) {
                    chip_cursor = 0;
                    state = STATE_MENU_RUN;
                    break;
                }
                DMA_CpuMemCopy1(tile_ptr, chip_animin_frames[chip_cursor], 42 * 256);
            }
            break;

        case STATE_MENU_RUN:
            timer++;
            if (timer == CHIP_RUN_TIMING) {
                timer = 0;
                chip_cursor++;
                if (chip_cursor == CHIP_RUN_NUMFRAMES) {
                    chip_cursor = 0;
                }
                DMA_CpuMemCopy1(tile_ptr, chip_run_frames[chip_cursor], 42 * 256);    
            }

            break;

    }
    //spawn a bg star every frame
    SPRITE_INFO *star = sprite_next();
    if (star != NULL) {
        menu_starcreate(star);
    }
    return 0;
}
