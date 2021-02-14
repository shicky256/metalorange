#include <assert.h>
#include <sega_dma.h>
#include <sega_mth.h>
#include <sega_scl.h>
#define _SPR2_
#include <sega_spr.h>

#include "cd.h"
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
static char *chip_animin_frames[CHIP_ANIMIN_NUMFRAMES] = {(char *)LWRAM + SCROLL_HEADER256,
                                                          (char *)LWRAM + SCROLL_HEADER256 + (1 * CHIP_FRAME_SIZE),
                                                          (char *)LWRAM + SCROLL_HEADER256,
                                                          (char *)LWRAM + SCROLL_HEADER256 + (2 * CHIP_FRAME_SIZE),
                                                          (char *)LWRAM + SCROLL_HEADER256 + (3 * CHIP_FRAME_SIZE)};
#define CHIP_RUN_NUMFRAMES (4)
#define CHIP_RUN_TIMING (13)
static char *chip_run_frames[CHIP_RUN_NUMFRAMES] = {(char *)LWRAM + SCROLL_HEADER256 + (4 * CHIP_FRAME_SIZE),
                                                    (char *)LWRAM + SCROLL_HEADER256 + (5 * CHIP_FRAME_SIZE),
                                                    (char *)LWRAM + SCROLL_HEADER256 + (6 * CHIP_FRAME_SIZE),
                                                    (char *)LWRAM + SCROLL_HEADER256 + (7 * CHIP_FRAME_SIZE)};

#define CHIP_ANIMOUT_NUMFRAMES (6)
static int chip_animout_timings[CHIP_ANIMOUT_NUMFRAMES] = {10, 10, 16, 32, 32, 32};
static char *chip_animout_frames[CHIP_ANIMOUT_NUMFRAMES] = {(char *)LWRAM + SCROLL_HEADER256 + (3 * CHIP_FRAME_SIZE),
                                                            (char *)LWRAM + SCROLL_HEADER256 + (2 * CHIP_FRAME_SIZE),
                                                            (char *)LWRAM + SCROLL_HEADER256,
                                                            (char *)LWRAM + SCROLL_HEADER256 + (1 * CHIP_FRAME_SIZE),
                                                            (char *)LWRAM + SCROLL_HEADER256,
                                                            (char *)LWRAM + SCROLL_HEADER256 + (8 * CHIP_FRAME_SIZE)};

static char *tile_ptr = ((char *)SCL_VDP2_VRAM_A1) + 256;
static char *font_ptr = ((char *)SCL_VDP2_VRAM_B1);
static Uint32 font_highlight[16];
static int timer;
static int menu_cursor = 0;
int cutscene = 1;

#define STAR_WIDTH (16)
#define STAR_HEIGHT (16)
int star_charno;
int star_num;

typedef struct {
    Fixed32 dx;
    Fixed32 dy;
} STAR_DATA;

static_assert(sizeof(STAR_DATA) <= SPRITE_DATA_SIZE, "Struct size too large");

typedef enum {
    STATE_MENU_INIT,
    STATE_MENU_FADEIN,
    STATE_MENU_ANIMIN,
    STATE_MENU_RUN,
    STATE_MENU_ANIMOUT,
    STATE_MENU_FADEOUT,
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
    // fix vdp2 config from intro
    SCL_SetColMixRate(SCL_NBG0, 0);
    //put NBG0 in a good location
    scroll_set(0, MTH_FIXED(-220), MTH_FIXED(-140));
    scroll_set(1, MTH_FIXED(0), MTH_FIXED(0));
    //load star gfx
    sprite_clear();
    star_charno = sprite_load("STARSPR.SPR", &star_num);
    SCL_SetPriority(SCL_SPR, 2); //put spr scroll layer under everything
    //load font gfx
    cd_load("MENUFONT.TLE", menu_buf);
    scroll_loadtile(menu_buf, font_ptr, SCL_NBG1, 0);  
    menu_print(5, 5, 0, "START GAME");
    // menu_print(5, 6, 0, "LOAD GAME");
    if (cutscene) {
        menu_print(5, 7, 0, "CUTSCENE: ON ");
    }
    else {
        menu_print(5, 7, 0, "CUTSCENE: OFF");
    }
    //load chip gfx
    cd_load("CHIPFRAM.TLE", menu_buf);
    scroll_loadtile(menu_buf, tile_ptr, SCL_NBG0, 0);
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
    sound_cdda(MENU_TRACK, 1);
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
    if ((star->x < MTH_FIXED(-32)) || (star->x > MTH_FIXED(SCROLL_LORES_X) ||
        (star->y < MTH_FIXED(-32)) || (star->y > MTH_FIXED(SCROLL_LORES_Y)))) {
        sprite_delete(star);
        return;
    }
    //acceleration
    STAR_DATA *star_data = (STAR_DATA *)star->data;
    star->x += MTH_Mul(star_data->dx, MAX(star->scale, MTH_FIXED(0.5)));
    star->y += MTH_Mul(star_data->dy, MAX(star->scale, MTH_FIXED(0.5)));

    //calculate distance using pythagorean theorem, use to calculate sprite scale
    Sint32 distance_x = (star->x >> 16) - (SCROLL_LORES_X >> 1);
    Sint32 distance_y = (star->y >> 16) - (SCROLL_LORES_Y >> 1);
    distance_x *= distance_x;
    distance_y *= distance_y;
    Uint32 distance = sqrt32((Uint32)distance_x + (Uint32)distance_y);
    star->scale = MTH_Div(MTH_IntToFixed(distance), MTH_FIXED(80));

    star->angle += MTH_FIXED(3);
}

static inline void menu_starcreate(SPRITE_INFO *star) {
    Fixed32 x = (Fixed32)MTH_GetRand() % MTH_FIXED(5) + MTH_FIXED(SCROLL_LORES_X >> 1);
    Fixed32 y = (Fixed32)MTH_GetRand() % MTH_FIXED(5) + MTH_FIXED(SCROLL_LORES_Y >> 1);
    int tile_num = star_charno + ((MTH_GetRand() >> 5) % star_num);
    sprite_make(tile_num, x, y, star);
    star->x_size = MTH_FIXED(STAR_WIDTH);
    star->y_size = MTH_FIXED(STAR_HEIGHT);
    Fixed32 angle = (Fixed32)(MTH_GetRand() % MTH_FIXED(360));
    angle -= MTH_FIXED(180);
    STAR_DATA *star_data = (STAR_DATA *)star->data;
    star_data->dx = MTH_Mul(MTH_FIXED(5), MTH_Cos(angle));
    star_data->dy = MTH_Mul(MTH_FIXED(5), MTH_Sin(angle));
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
                    //use first frame for fadeout
                    DMA_CpuMemCopy1(tile_ptr, chip_animin_frames[0], 42 * 256);
                    state = STATE_MENU_FADEOUT;
                    break;
                }
                DMA_CpuMemCopy1(tile_ptr, chip_animout_frames[chip_cursor], 42 * 256);
            }
            break;

        case STATE_MENU_FADEOUT:
            timer++;
            //wait a second before fading out
            if (timer == 60) {
                SclRgb start, end;
                start.red = 0; start.green = 0; start.blue = 0;
                end.red = -255; end.green = -255; end.blue = -255;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
            }
            else if (timer == 90) {
                sprite_deleteall();
                state = STATE_MENU_DONE;
            }
            break;

        case STATE_MENU_DONE:
            state = STATE_MENU_INIT;
            return 1;
            break;

    }
    if (state == STATE_MENU_RUN) {
        //handle menu movement
        if ((PadData1E & PAD_U) && (menu_cursor > 0)) {
            menu_cursor = 0;
            // pcm_play(1, PCM_SEMI, 6);
            sound_play(SOUND_SELECT);
        }
        if ((PadData1E & PAD_D) && (menu_cursor < 2)) {
            menu_cursor = 2;
            // pcm_play(1, PCM_SEMI, 6);
            sound_play(SOUND_SELECT);
        }
        //start game
        if ((menu_cursor == 0) && (PadData1E & (PAD_S | PAD_A | PAD_B | PAD_C))) {
            chip_cursor = 0;
            timer = 0;
            state = STATE_MENU_ANIMOUT;
            // pcm_play(2, PCM_SEMI, 6);
            sound_play(SOUND_START);
        }
        //handle cutscene toggle
        if ((menu_cursor == 2) && (PadData1E & (PAD_S | PAD_A | PAD_B | PAD_C))) {
            cutscene ^= 1;
            // pcm_play(2, PCM_SEMI, 6);
            sound_play(SOUND_START);
        }
        //cycle palette for selected item
        menu_palcycle();
        if (menu_cursor == 0) { menu_print(5, 5, 1, "START GAME"); }
        else menu_print(5, 5, 0, "START GAME");

        // if (menu_cursor == 1) { menu_print(5, 6, 1, "LOAD GAME"); }
        // else { menu_print(5, 6, 0, "LOAD GAME"); }

        if (cutscene) {
            if (menu_cursor == 2) { menu_print(5, 7, 1, "CUTSCENE: ON "); }
            else { menu_print(5, 7, 0, "CUTSCENE: ON "); }
        }

        else {
            if (menu_cursor == 2) { menu_print(5, 7, 1, "CUTSCENE: OFF"); }
            else { menu_print(5, 7, 0, "CUTSCENE: OFF"); }
        }
    }
    if ((state >= STATE_MENU_ANIMIN) && (state < STATE_MENU_DONE)) {
        //spawn a bg star everyframe
        SPRITE_INFO *star = sprite_next();
        if (star != NULL) {
            menu_starcreate(star);
        }
    }
    return 0;
}
