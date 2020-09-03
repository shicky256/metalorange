#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>
#define _SPR2_
#include <SEGA_SPR.H>

#include "cd.h"
#include "graphicrefs.h"
#include "menu.h"
#include "pcmsys.h"
#include "sound.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

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

#define CHIP_ANIMOUT_NUMFRAMES (6)
static int chip_animout_timings[CHIP_ANIMOUT_NUMFRAMES] = {10, 10, 16, 32, 32, 32};
static char *chip_animout_frames[CHIP_ANIMOUT_NUMFRAMES] = {(char *)LWRAM + 3 * CHIP_FRAME_SIZE,
                                                            (char *)LWRAM + 2 * CHIP_FRAME_SIZE,
                                                            (char *)LWRAM,
                                                            (char *)LWRAM + 1 * CHIP_FRAME_SIZE,
                                                            (char *)LWRAM,
                                                            (char *)LWRAM + 8 * CHIP_FRAME_SIZE};

static char *tile_ptr = ((char *)SCL_VDP2_VRAM_A1) + 256;
static char *font_ptr = ((char *)SCL_VDP2_VRAM_B1);
static Uint32 font_highlight[16];
static int timer;
static int menu_cursor = 0;
int nudity = 0;

#define STAR_CHARNO (font_num)

typedef enum {
    STATE_MENU_INIT,
    STATE_MENU_FADEIN,
    STATE_MENU_ANIMIN,
    STATE_MENU_RUN,
    STATE_MENU_ANIMOUT,
    STATE_MENU_DONE
} MENU_STATE;

static int state = STATE_MENU_INIT;

static void menu_print(int x, int y, int highlight, char *str) {
    Uint16 *text_ptr = MAP_PTR(1);
    char ch;
    int counter = 0;
    if (highlight) {
        while ((ch = *str++)) {
            text_ptr[y * 32 + x + counter++] = (1 << 12) | (ch - 32); //set palette #1
        }
    }
    else {
        while ((ch = *str++)) {
            text_ptr[y * 32 + x + counter++] = ch - 32;
        }
    }
}

static int menu_cycleiter(Uint8 r_val, Uint8 g_val, Uint8 b_val) {
    static Uint8 r = 255;
    static Uint8 g = 255;
    static Uint8 b = 255;

    if (r > r_val) r--;
    if (r < r_val) r++;

    if (g > g_val) g--;
    if (g < g_val) g++;

    if (b > b_val) b--;
    if (b < b_val) b++;

    Uint32 r_mod = r;
    Uint32 g_mod = g;
    Uint32 b_mod = b;
    for (int i = 8; i > 0; i--) {
        font_highlight[i] = r_mod | (g_mod << 8) | (b_mod << 16);
        r_mod *= 9; r_mod /= 10;
        g_mod *= 9; g_mod /= 10;
        b_mod *= 9; b_mod /= 10;
    }
    font_highlight[0] = 0;
    SCL_SetColRam(SCL_NBG1, 16, 16, font_highlight);
    return (r == r_val) && (g == g_val) && (b == b_val);
}

static void menu_palcycle() {
    static int pal_cursor = 0;

    switch(pal_cursor) {
        case 0: //red
            if (menu_cycleiter(255, 0, 0)) pal_cursor++;
            break;
        case 1: //green
            if (menu_cycleiter(0, 255, 0)) pal_cursor++;
            break;
        case 2: //blue
            if (menu_cycleiter(0, 0, 255)) pal_cursor++;
            break;
        case 3: //yellow
            if (menu_cycleiter(255, 255, 0)) pal_cursor++;
            break;
        case 4: //purple
            if (menu_cycleiter(80, 0, 80)) pal_cursor++;
            break;
        case 5: //aqua
            if (menu_cycleiter(0, 255, 255)) pal_cursor = 0;
            break;
    }
}

static inline void menu_init() {
    char *menu_buf = (char *)LWRAM;
    Uint16 *map_ptr = MAP_PTR(0);

    scroll_lores();
    SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2, -255, -255, -255);
    scroll_clearmaps();
    //put NBG0 in a good location
    scroll_set(0, MTH_FIXED(-220), MTH_FIXED(-140));
    scroll_set(1, MTH_FIXED(0), MTH_FIXED(0));
    //load star gfx
    cd_load_nosize(starspr_name, sprite_buf);
    SCL_SetColRam(SCL_SPR, 16, 16, starspr_pal);
	for (int i = 0; i < starspr_num; i++) {
		SPR_2SetChar(i + STAR_CHARNO, COLOR_0, 16, starspr_width, starspr_height, sprite_buf + (i * starspr_size));
	}
    SCL_SetPriority(SCL_SPR, 2); //put spr scroll layer under everything
    //load font gfx
    cd_load_nosize(menufont_name, menu_buf);
    DMA_CpuMemCopy1(font_ptr, menu_buf, 128 * menufont_num);
    SCL_SetColRam(SCL_NBG1, 0, 16, menufont_pal);
    menu_print(6, 5, 0, "START GAME");
    menu_print(6, 6, 0, "LOAD GAME");
    menu_print(6, 7, 0, "NUDITY: OFF");
    //load chip gfx
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
    sound_cdda(4, 1);
}

//Algorithm by Tristan Muntsinger
Uint32 sqrt32(Uint32 n)  
{  
    unsigned int c = 0x8000;  
    unsigned int g = 0x8000;  
    
    for(;;) {  
        if(g*g > n)  
            g ^= c;  
        c >>= 1;  
        if(c == 0)  
            return g;  
        g |= c;  
    }  
}  

static void menu_starmove(SPRITE_INFO *star) {
    if ((star->x < MTH_FIXED(0)) || (star->x > MTH_FIXED(SCROLL_LORES_X) ||
        (star->y < MTH_FIXED(0)) || (star->y > MTH_FIXED(SCROLL_LORES_Y)))) {
        sprite_delete(star);
        return;
    }
    //acceleration
    star->x += MTH_Mul(star->dx, MAX(star->scale, MTH_FIXED(0.5)));
    star->y += MTH_Mul(star->dy, MAX(star->scale, MTH_FIXED(0.5)));
    //calculate distance using pythagorean theorem, use to calculate sprite scale
    Sint32 distance_x = (star->x >> 16) - (SCROLL_LORES_X >> 1);
    Sint32 distance_y = (star->y >> 16) - (SCROLL_LORES_Y >> 1);
    distance_x *= distance_x;
    distance_y *= distance_y;
    Uint32 distance = sqrt32((Uint32)distance_x + (Uint32)distance_y);
    star->scale = MTH_Div(MTH_IntToFixed(distance), MTH_FIXED(80));
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
    static int frames = 0;
    frames++;    
    switch (state) {
        case STATE_MENU_INIT:
            menu_init();
            timer = 0;
            chip_cursor = 0;
            //fade in screen
            SclRgb start, end;
            start.red = -255; start.green = -255; start.blue = -255;
            end.red = 0; end.green = 0; end.blue = 0;
            SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);

            state = STATE_MENU_FADEIN;
            break;

        case STATE_MENU_FADEIN:
            timer++;
            if (timer == 30) {
                timer = 0;
                state = STATE_MENU_ANIMIN;
            }
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
        
        case STATE_MENU_ANIMOUT:
            timer++;
            if (timer == chip_animout_timings[chip_cursor]) {
                timer = 0;
                chip_cursor++;
                if (chip_cursor == CHIP_ANIMOUT_NUMFRAMES) {
                    chip_cursor = 0;
                    state = STATE_MENU_DONE;
                    //use first frame for fadeout
                    DMA_CpuMemCopy1(tile_ptr, (Uint8 *)LWRAM, 42 * 256);
                    break;
                }
                DMA_CpuMemCopy1(tile_ptr, chip_animout_frames[chip_cursor], 42 * 256);
            }
            break;

        case STATE_MENU_DONE:
            break;

    }
    if (state == STATE_MENU_RUN) {
        //handle menu movement
        if ((PadData1E & PAD_U) && (menu_cursor > 0)) {
            menu_cursor--;
            pcm_play(1, PCM_SEMI, 6);
        }
        if ((PadData1E & PAD_D) && (menu_cursor < 2)) {
            menu_cursor++;
            pcm_play(1, PCM_SEMI, 6);
        }
        //start game
        if ((menu_cursor == 0) && (PadData1E & (PAD_S | PAD_A | PAD_B | PAD_C))) {
            chip_cursor = 0;
            timer = 0;
            state = STATE_MENU_ANIMOUT;
            pcm_play(2, PCM_SEMI, 6);
        }
        //handle nudity toggle
        if ((menu_cursor == 2) && (PadData1E & (PAD_S | PAD_A | PAD_B | PAD_C))) {
            nudity ^= 1;
            pcm_play(2, PCM_SEMI, 6);
        }
        //cycle palette for selected item
        menu_palcycle();
        if (menu_cursor == 0) { menu_print(6, 5, 1, "START GAME"); }
        else menu_print(6, 5, 0, "START GAME");

        if (menu_cursor == 1) { menu_print(6, 6, 1, "LOAD GAME"); }
        else { menu_print(6, 6, 0, "LOAD GAME"); }

        if (nudity) {
            if (menu_cursor == 2) { menu_print(6, 7, 1, "NUDITY: ON "); }
            else { menu_print(6, 7, 0, "NUDITY: ON "); }
        }

        else {
            if (menu_cursor == 2) { menu_print(6, 7, 1, "NUDITY: OFF"); }
            else { menu_print(6, 7, 0, "NUDITY: OFF"); }
        }
    }
    if (state >= STATE_MENU_ANIMIN) {
        //spawn a bg star everyframe
        SPRITE_INFO *star = sprite_next();
        if (star != NULL) {
            menu_starcreate(star);
        }
    }
    return 0;
}
