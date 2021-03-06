#include <sega_dma.h>
#include <sega_scl.h>
#define _SPR2_
#include <sega_spr.h>
#include <string.h>

#include "cd.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "intro.h"
#include "print.h"
#include "vblank.h"

typedef enum {
    STATE_INTRO_INIT = 0,
    STATE_INTRO_LOADGIRL,
    STATE_INTRO_DISPGIRL,
    STATE_INTRO_MOVEGIRL,
    STATE_INTRO_TEXT,
    STATE_INTRO_FADEGIRL,
    STATE_INTRO_LOADTITLE,
    STATE_INTRO_SHOWTITLE,
    STATE_INTRO_LOADMETALORANGE,
    STATE_INTRO_SHOWMETALORANGE,
    STATE_INTRO_MOVEMETALORANGE,
    STATE_INTRO_LOADCYBERBLOCK,
    STATE_INTRO_SHOWCYBERBLOCK,
    STATE_INTRO_FADEOUT,
} INTRO_STATE;

#define SCREEN_X (704)
#define GIRL_X (272)
#define MOVE_SPEED (MTH_FIXED(7))
#define METALORANGE_Y (-100)

static int state = STATE_INTRO_INIT;
static char *girls[5];
static char *tile_ptr;
static char *title_gfx_ptr;
static char *metalorange_ptr;
static char *cyberblock_ptr;

static int cursor; //where we are in the girl array
static int frames; //frame counter
static Fixed32 girl_xpos; //girl's x position
static Fixed32 star_x = 0, star_y = 0;
static Fixed32 star_angle = 0;
static SclLineparam line_param;
static Fixed32 titletext_pos;


#define TEXT_LOAD (0)
#define TEXT_SHOW (1)
static int text_mode;
static int text_num;
static int text_frame;

static Fixed32 linescroll_angle;
static int linescroll_multiplier;

// intro credits strings
// credit pseudonyms:
// Kochan = Kotaro Horie (�x�]�F���Y)
// Ath Nagano = Atsuya Yagano (����֖玁)
// H. Yanagi = Hirohiko Yanagi (���Ђ�Ђ�)
// N. Nakajima = ?
// JKL Furukawa = Yoshio Furukawa (�Ð�`�Y)
// MKM Hirota = Hitoshi Sakimoto (�茳�m)
// Rezon Iwata = Masuharu Iwata (��c����)
// Waiendee = ?

// Custom info:
// "If you're talking about Cybertech Custom, it looks like the company closed down around the year 2000.
// After the company's entry into the console market was a huge failure, the game designers and programmers all quit.
// They couldn't do proper development, so they outsourced the porting of the Windows versions to a third party, and ended their activities.
// The representative of the company, Hirohiko Yanagi, is still active in the erotic doujin business (although his designs are different now).
// From a doujin software shop to a manufacturer, he became so absorbed in making doujin magazines that he ended up going out of business."

static char program_str[] = "PROGRAM\nKOCHAN";
static char design_str[] = "GAME DESIGN\nATH NAGANO";
static char graphic_str[] = "GRAPHIC\n       CREATE\n  H. YANAGI\n  N. NAKAJIMA";
static char character_str[] = "CHARACTER\n        CREATE\n   H. YANAGI\n    KOCHAN";
static char music_str[] = "MUSIC COMPOSE\nJKL FURUKAWA\n MKM HIROTA\n REZON IWATA";
static char sound_str[] = "SOUND EFFECT\n  WAIENDEE";
static char opm_str[] = " OPM ARRANGE\nJKL FURUKAWA\n  WAIENDEE";
static char midi_str[] = "MIDI ARRANGE\n MKM HIROTA\nJKL FURUKAWA";
static char direct_str[] = "  DIRECT\nATH NAGANO";
static char produce_str[] = "PRODUCE\nH. YANAGI";

static int introfont_widths[] = {
    8, 7, 11, 14, 18, 14, 19, 7, 11, 11, 14, 14, 7, 10, 7, 6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 7, 7, 9, 14, 9, 13,
	27, 21, 22, 17, 23, 22, 19, 21, 25, 13, 14, 23, 21, 27, 22, 20,
	20, 20, 21, 16, 19, 22, 21, 30, 23, 22, 17, 10, 9, 11, 8, 23
};

int introfont_charno;
static Uint32 introfont_curpal[16];

static char *intro_strs[] = {program_str, design_str, graphic_str, character_str, music_str,
    sound_str, opm_str, midi_str, direct_str, produce_str};
static inline void intro_init() {
    char *img_buf = (char *)LWRAM;
    char *dest_buf = img_buf;
    Uint16 *map_ptr = MAP_PTR(0);
    //initialize tilemap
    int counter = 1;
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 32; j++) {
            if (j < 17) {
                map_ptr[i * 32 + j] = 2 * counter++;
            }
            else {
                map_ptr[i * 32 + j] = 0;
            }
        }
    }

    scroll_hires();
    scroll_set(0, MTH_FIXED(0), MTH_FIXED(0));
    // set layer priorities
	SCL_SetPriority(SCL_SPR, 7);
	SCL_SetPriority(SCL_SP1, 7);
	SCL_SetPriority(SCL_NBG0, 6);
	SCL_SetPriority(SCL_NBG1, 5);
	SCL_SetPriority(SCL_NBG2, 4);
	SCL_SetPriority(SCL_NBG3, 3);
    
    // load font for intro
    sprite_clear();
    introfont_charno = sprite_load("INTROFON.SPR", NULL);

    //load all 5 images
    int size;
    girls[0] = dest_buf;
    size = cd_load("GIRL1.TLE", dest_buf);
    dest_buf += size;
    girls[1] = dest_buf;
    size = cd_load("GIRL2.TLE", dest_buf);
    dest_buf += size;
    girls[2] = dest_buf;
    size = cd_load("GIRL3.TLE", dest_buf);
    dest_buf += size;
    girls[3] = dest_buf;
    size = cd_load("GIRL4.TLE", dest_buf);
    dest_buf += size;
    girls[4] = dest_buf;
    size = cd_load("GIRL5.TLE", dest_buf);
    dest_buf += size;

    //load star gfx & tilemaps
    // cd_load(stars_name, dest_buf);
    // DMA_CpuMemCopy1((void *)SCL_VDP2_VRAM_B1, dest_buf, 128 * stars_num);
    cd_load("STARS.TLE", dest_buf);
    scroll_loadtile(dest_buf, (void *)SCL_VDP2_VRAM_B1, SCL_NBG1, 0);

    int map_x, map_y;
    char *map_data;

    //near tilemap
    cd_load("STARNEAR.MAP", dest_buf);
    map_data = scroll_mapptr(dest_buf, &map_x, &map_y);
    memcpy(MAP_PTR(1), map_data, map_x * map_y * 2);

    //mid tilemap
    cd_load("STARMID.MAP", dest_buf);
    map_data = scroll_mapptr(dest_buf, &map_x, &map_y);
    memcpy(MAP_PTR(2), map_data, map_x * map_y * 2);
    // SCL_SetColRam(SCL_NBG2, 0, 16, stars_pal);
    //far tilemap
    cd_load("STARFAR.MAP", dest_buf);
    map_data = scroll_mapptr(dest_buf, &map_x, &map_y);
    memcpy(MAP_PTR(3), map_data, map_x * map_y * 2);
    // SCL_SetColRam(SCL_NBG3, 0, 16, stars_pal);
    
    //load title screen gfx
    size = cd_load("TITLE.TLE", dest_buf);
    title_gfx_ptr = dest_buf;
    dest_buf += size;
    size = cd_load("METALORA.TLE", dest_buf);
    metalorange_ptr = dest_buf;
    dest_buf += size;
    size = cd_load("CYBERBLO.TLE", dest_buf);
    cyberblock_ptr = dest_buf;

    tile_ptr = (char *)SCL_VDP2_VRAM_A1;
    for (int i = 0; i < 256; i++) {
        tile_ptr[i] = 0;
    }
    tile_ptr += 256;

    // init vars
    text_mode = TEXT_LOAD;
    text_num = 0;
    text_frame = 0;
    linescroll_angle = 0;
    linescroll_multiplier = MTH_FIXED(16);
}

static void intro_print(int x, int y, char *str) {
    int start_x = x;
    SPRITE_INFO letter;
    char ch;

    while ((ch = *str) != '\0') {
        if (ch == '\n') {
            y += 32;
            x = start_x;
            str++;
            continue;
        }

        ch -= 32; //font starts with the space character
        sprite_make(ch + introfont_charno, MTH_IntToFixed(x), MTH_IntToFixed(y), &letter);
        sprite_draw(&letter);
        x += introfont_widths[(int)ch];
        str++;
    }
}

static inline int intro_disptext(int girl) {
    int x_pos, y_pos;
    if ((girl & 1) == 0) { //even girls are on the right
        x_pos = 120;
        y_pos = 120;
    }
    else { //odd girls are on the left
        x_pos = 360;
        y_pos = 120;
    }
    if (text_mode == TEXT_LOAD) {
        text_frame = 0;
        text_mode = TEXT_SHOW;
        // clear out text palette
        for (int i = 0; i < sizeof(introfont_curpal) / sizeof(introfont_curpal[0]); i++) {
            introfont_curpal[i] = 0;
        }
        SCL_SetColRam(SCL_SPR, 0, 16, introfont_curpal);
    }
    else {
        text_frame++;
        if (text_frame == 180) {
            text_num++;
            text_mode = TEXT_LOAD;
            if (text_num == 2) {
                text_num = 0;
                return 1;
            }
        }
        // "draw down" palette effect
        for (int i = 0; i < sizeof(introfont_curpal) / sizeof(introfont_curpal[0]); i++) {
            if (introfont_curpal[i] == 0) {
                introfont_curpal[i] = 0xFF0000;
                break;
            }
        }
        SCL_SetColRam(SCL_SPR, 0, 16, introfont_curpal);
    }

    intro_print(x_pos, y_pos, intro_strs[girl * 2 + text_num]);
    return 0;
}

static inline void intro_drawpolys(int num) {
    XyInt rect[4];
    Uint32 black = 0;
    SCL_SetColRam(SCL_SPR, 2, 1, &black);
    switch (num) {
        case 5:
            rect[0].x = 140; rect[0].y = 0; //top left
            rect[1].x = 140 + 72; rect[1].y = 0; //top right
            rect[2].x = 140 + 72; rect[2].y = 240; //bottom right
            rect[3].x = 140; rect[3].y = 240; //bottom left
            SPR_2Polygon(0, SPD_DISABLE, 2, rect, NO_GOUR);
            //fall through to next case

        case 4:
            rect[0].x = 282; rect[0].y = 0;
            rect[1].x = 282 + 72; rect[1].y = 0;
            rect[2].x = 282 + 72; rect[2].y = 240;
            rect[3].x = 282; rect[3].y = 240;
            SPR_2Polygon(0, SPD_DISABLE, 2, rect, NO_GOUR);
            //fall through to next case

        case 3:
            rect[0].x = 0; rect[0].y = 0;
            rect[1].x = 72; rect[1].y = 0;
            rect[2].x = 72; rect[2].y = 240;
            rect[3].x = 0; rect[3].y = 240;
            SPR_2Polygon(0, SPD_DISABLE, 2, rect, NO_GOUR);
            //fall through to next case

        case 2:
            rect[0].x = 210; rect[0].y = 0;
            rect[1].x = 210 + 72; rect[1].y = 0;
            rect[2].x = 210 + 72; rect[2].y = 240;
            rect[3].x = 210; rect[3].y = 240;
            SPR_2Polygon(0, SPD_DISABLE, 2, rect, NO_GOUR);
            //fall through to next case

        case 1:
            rect[0].x = 72; rect[0].y = 0;
            rect[1].x = 72 + 72; rect[1].y = 0;
            rect[2].x = 72 + 72; rect[2].y = 240;
            rect[3].x = 72; rect[3].y = 240;
            SPR_2Polygon(0, SPD_DISABLE, 2, rect, NO_GOUR);
    }    
}

static inline int intro_linescroll() {
    linescroll_angle += MTH_FIXED(2);
    linescroll_multiplier -= MTH_FIXED(0.08);
    if (linescroll_multiplier < MTH_FIXED(4.5)) { //otherwise it spends too much time wiggling in the middle, which looks shitty
        linescroll_multiplier = 0;
    }
    for (int i = 0; i < 48; i++) {
        Fixed32 curr_angle = linescroll_angle + MTH_IntToFixed(i * 2);
        curr_angle %= MTH_FIXED(360);
        curr_angle -= MTH_FIXED(180);
        // if (curr_angle > MTH_FIXED(180)) {
        //     curr_angle -= MTH_FIXED(360);
        // }
        if (i & 1) {
            line_param.line_tbl[i - METALORANGE_Y].h = MTH_Mul(MTH_Sin(curr_angle), linescroll_multiplier);
        }
        else {
            line_param.line_tbl[i - METALORANGE_Y].h = MTH_Mul(MTH_Sin(curr_angle), -linescroll_multiplier);
        }
    }
    SCL_Open(SCL_NBG1);
    SCL_SetLineParam(&line_param);
    SCL_Close();
    if (linescroll_multiplier == 0) {
        return 1;
    }
    return 0;
}

int intro_run() {
    Uint16 *map_ptr;
    Uint16 count;

    switch(state) {
        case STATE_INTRO_INIT:
            SCL_SetColMixRate(SCL_NBG0, 31);

            intro_init();
            sound_cdda(INTRO_TRACK, 1);
            cursor = 0;
            state = STATE_INTRO_LOADGIRL;
            break;
        
        case STATE_INTRO_LOADGIRL:
            SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, 0, 0, 0);
            scroll_loadtile(girls[cursor], tile_ptr, SCL_NBG0, 0);
            girl_xpos = MTH_FIXED(-((SCREEN_X / 2) - (GIRL_X / 2)));
            scroll_set(0, girl_xpos, MTH_FIXED(0)); //center image
            // SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end); //fade in image
            SCL_SetAutoColMix(SCL_NBG0, 1, 30, 31, 0);
            frames = 0;
            state = STATE_INTRO_DISPGIRL;
            break;
        
        case STATE_INTRO_DISPGIRL:
            frames++;
            if (frames == 90) {
                state = STATE_INTRO_MOVEGIRL;
            }
            break;

        case STATE_INTRO_MOVEGIRL:
            if ((cursor & 1) == 0) {
                girl_xpos -= MOVE_SPEED;
                if (girl_xpos < MTH_FIXED(-(SCREEN_X - GIRL_X))) {
                    girl_xpos = MTH_FIXED(-(SCREEN_X - GIRL_X) - 1);
                    state = STATE_INTRO_TEXT;
                }
            }
            else {
                girl_xpos += MOVE_SPEED;
                if (girl_xpos > MTH_FIXED(1)) {
                    girl_xpos = MTH_FIXED(1);
                    state = STATE_INTRO_TEXT;
                }
            }
            scroll_set(0, girl_xpos, MTH_FIXED(0));
            break;

        case STATE_INTRO_TEXT:
            if (intro_disptext(cursor)) {
                // SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &end, &start);
                SCL_SetAutoColMix(SCL_NBG0, 1, 30, 0, 31);
                frames = 0;
                state = STATE_INTRO_FADEGIRL;                
            }
            break;

        case STATE_INTRO_FADEGIRL:
            frames++;
            if (frames == 60) {
                cursor++;
                if (cursor == (sizeof(girls) / sizeof(girls[0]))) {
                    state = STATE_INTRO_LOADTITLE;
                }
                else {
                    state = STATE_INTRO_LOADGIRL;
                }
            }
            break;

        case STATE_INTRO_LOADTITLE:
            // SCL_SetColRam(SCL_NBG0, 0, 256, title_pal);
            SCL_SetColMixRate(SCL_NBG0, 0); //make screen opaque
            SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255); // black out the screen
            // DMA_CpuMemCopy1(tile_ptr, title_gfx_ptr, 256 * title_num);
            // memcpy(tile_ptr, title_gfx_ptr, 256 * title_num);
            scroll_loadtile(title_gfx_ptr, tile_ptr, SCL_NBG0, 0);
            scroll_set(0, MTH_FIXED(0), MTH_FIXED(0)); //reset map
            count = 2;
            map_ptr = MAP_PTR(0);
            for (int i = 0; i < (240 / 16); i++) {
                for (int j = 0; j < (352 / 16); j++) {
                    map_ptr[i * 32 + j] = count;
                    count += 2;
                }
            }
            //zero out the other maps to avoid artifacts
            map_ptr = MAP_PTR(1);
            for (int i = 0; i < 32 * 32; i++) {
                map_ptr[i] = 0;
            }
            map_ptr = MAP_PTR(2);
            for (int i = 0; i < 32 * 32; i++) {
                map_ptr[i] = 0;
            }
            map_ptr = MAP_PTR(3);
            for (int i = 0; i < 32 * 32; i++) {
                map_ptr[i] = 0;
            }
            intro_drawpolys(5); //black out the screen
            scroll_lores();
            cursor = 5;
            frames = 0;
            state = STATE_INTRO_SHOWTITLE;
            //fall through

        case STATE_INTRO_SHOWTITLE:
            frames++;
            if (frames == 12) {
                cursor--;
                SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, 0, 0, 0); // clear screen blackout
                frames = 0;
                if (cursor == 0) {
                    state = STATE_INTRO_LOADMETALORANGE;
                }
            }
            intro_drawpolys(cursor);
            break;
        
        case STATE_INTRO_LOADMETALORANGE:
            // SCL_SetColRam(SCL_NBG1, 0, 16, metalorange_pal); //load palette
            // DMA_CpuMemCopy1((void *)(SCL_VDP2_VRAM_B1 + 128), metalorange_ptr, 128 * metalorange_num); //skip first tile
            scroll_loadtile(metalorange_ptr, (void *)(SCL_VDP2_VRAM_B1 + 128), SCL_NBG1, 0);
            SCL_SetPriority(SCL_NBG1, 7); //put nbg1 on top of nbg0
            scroll_set(1, MTH_FIXED(-((352 / 2) - (256 / 2))), MTH_FIXED(METALORANGE_Y)); //reset position
            map_ptr = MAP_PTR(1);
            count = 1;
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    if (i < 3 && j < 16) {
                        map_ptr[i * 32 + j] = count++;
                    }
                    else {
                        map_ptr[i * 32 + j] = 0;
                    }
                }
            }
            SCL_InitLineParamTb(&line_param);
            line_param.h_enbl = ON;
            line_param.line_addr = SCL_VDP2_VRAM_B1 + 0x1f000; //b1 + 124kb
            line_param.interval = SCL_1_LINE;
            state = STATE_INTRO_SHOWMETALORANGE;
            break;
        
        case STATE_INTRO_SHOWMETALORANGE:
            if (intro_linescroll()) {
                titletext_pos = MTH_FIXED(METALORANGE_Y);
                state = STATE_INTRO_MOVEMETALORANGE;
            }
            break;
        case STATE_INTRO_MOVEMETALORANGE:
            titletext_pos -= MTH_FIXED(1);
            scroll_set(1, MTH_FIXED(-((352 / 2) - (256 / 2))), titletext_pos);
            if (titletext_pos <= MTH_FIXED(-140)) {
                state = STATE_INTRO_LOADCYBERBLOCK;
            }
            break;

        case STATE_INTRO_LOADCYBERBLOCK:
            SCL_SetColMixRate(SCL_NBG2, 31); //make bg2 transparent
            // SCL_SetColRam(SCL_NBG1, 16, 16, cyberblock_pal);
            //skip 1st blank tile & all the "metal orange" text tiles
            int metalorange_size;
            scroll_tileptr(metalorange_ptr, &metalorange_size);
            char *cyberblock_dest = (void *)((SCL_VDP2_VRAM_B1 + 128) + metalorange_size);
            scroll_loadtile(cyberblock_ptr, cyberblock_dest, SCL_NBG1, 16);
            SCL_SetPriority(SCL_NBG2, 7); //put on top
            scroll_set(2, MTH_FIXED(-((352 / 2) - (192 / 2))), MTH_FIXED(-120));
            map_ptr = MAP_PTR(2);
            count = (metalorange_size / 128) + 1;
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    if (i < 2 && j < 12) {
                        map_ptr[i * 32 + j] = count++ | (1 << 12);
                    }
                    else {
                        map_ptr[i * 32 + j] = 0;
                    }
                }
            }
            SCL_SetAutoColMix(SCL_NBG2, 1, 30, 31, 0);
            state = STATE_INTRO_SHOWCYBERBLOCK;
            break;
        
        case STATE_INTRO_SHOWCYBERBLOCK:
            break;

        case STATE_INTRO_FADEOUT:
            frames++;
            if (frames == 30) {
                state = STATE_INTRO_INIT;
                return 1;
            }
            break;       
    }
    //move stars
    if (state < STATE_INTRO_LOADTITLE) {
        star_angle += MTH_FIXED(0.1);
        if (star_angle > MTH_FIXED(180)) {
            star_angle = MTH_FIXED(-180);
        }
        star_x += MTH_Mul(MTH_Cos(star_angle), MTH_FIXED(10));
        star_y += MTH_Mul(MTH_Sin(star_angle), MTH_FIXED(10));

        scroll_set(1, star_x, star_y);
        scroll_set(2, star_x >> 1, star_y >> 1);
        scroll_set(3, star_x >> 2, star_y >> 2);
    }

    //do fadeout
    if (PadData1E & PAD_S) {
        SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2, 0, 0, 0);
        SclRgb start, end;
        start.red = 0; start.green = 0; start.blue = 0;
        end.red = -255; end.green = -255; end.blue = -255;
        SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
        frames = 0;

        state = STATE_INTRO_FADEOUT;
    }
    return 0;
}
