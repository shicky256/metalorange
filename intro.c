#include <SEGA_DMA.H>
#include <SEGA_SCL.H>

#include "cd.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "intro.h"
#include "print.h"

typedef enum {
    STATE_INTRO_INIT = 0,
    STATE_INTRO_LOADGIRL,
    STATE_INTRO_DISPGIRL,
    STATE_INTRO_MOVEGIRL,
    STATE_INTRO_TEXT,
    STATE_INTRO_FADEGIRL,
    STATE_INTRO_END
} INTRO_STATE;

#define SCREEN_X (704)
#define GIRL_X (272)
#define MOVE_SPEED (MTH_FIXED(6))

static int state = STATE_INTRO_INIT;
static char *girls[5];
static Uint32 *girl_pal[5] = {girl1_pal, girl2_pal, girl3_pal, girl4_pal, girl5_pal};
static char *tile_ptr = (char *)SCL_VDP2_VRAM_A1;

static int cursor; //where we are in the girl array
static int frames; //frame counter
static Fixed32 girl_xpos; //girl's x position
static Fixed32 star_x = 0, star_y = 0;
static Fixed32 star_angle = 0;

//intro credits strings
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

static char *intro_strs[] = {program_str, design_str, graphic_str, character_str, music_str,
    sound_str, opm_str, midi_str, direct_str, produce_str};

static inline void intro_init() {
    char *img_buf = (char *)LWRAM;
    char *dest_buf = img_buf;
    Uint16 *map_ptr = VRAM_PTR(0);
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

    scroll_set(0, MTH_FIXED(0), MTH_FIXED(0));
    //load all 5 images
    girls[0] = dest_buf;
    cd_load_nosize(girl1_name, dest_buf);
    dest_buf += 256 * girl1_num;
    girls[1] = dest_buf;
    cd_load_nosize(girl2_name, dest_buf);
    dest_buf += 256 * girl2_num;
    girls[2] = dest_buf;
    cd_load_nosize(girl3_name, dest_buf);
    dest_buf += 256 * girl3_num;
    girls[3] = dest_buf;
    cd_load_nosize(girl4_name, dest_buf);
    dest_buf += 256 * girl4_num;
    girls[4] = dest_buf;
    cd_load_nosize(girl5_name, dest_buf);
    dest_buf += 256 * girl5_num;

    //load star gfx & tilemaps
    cd_load_nosize(stars_name, dest_buf);
    DMA_CpuMemCopy1((void *)SCL_VDP2_VRAM_B1, dest_buf, 128 * stars_num);
    //near tilemap
    cd_load_nosize(starnear_name, dest_buf);
    DMA_CpuMemCopy2(VRAM_PTR(1), dest_buf, 32 * 32);
    SCL_SetColRam(SCL_NBG1, 0, 16, stars_pal);
    //mid tilemap
    cd_load_nosize(starmid_name, dest_buf);
    DMA_CpuMemCopy2(VRAM_PTR(2), dest_buf, 32 * 32);
    // SCL_SetColRam(SCL_NBG2, 0, 16, stars_pal);
    //far tilemap
    cd_load_nosize(starfar_name, dest_buf);
    DMA_CpuMemCopy2(VRAM_PTR(3), dest_buf, 32 * 32);
    // SCL_SetColRam(SCL_NBG3, 0, 16, stars_pal);    


    for (int i = 0; i < 256; i++) {
        tile_ptr[i] = 0;
    }
    tile_ptr += 256;
}

#define TEXT_LOAD (0)
#define TEXT_SHOW (1)

static int intro_disptext(int girl) {
    static int text_mode = TEXT_LOAD;
    static int text_num = 0;
    static int frame;
    int row, col;

    if (text_mode == TEXT_LOAD) {
        if ((girl & 1) == 0) { //even girls are on the right
            row = 20;
            col = 10;
        }
        else { //odd girls are on the left
            row = 20;
            col = 50;
        }
        print_init();
        print_string(intro_strs[girl * 2 + text_num], row, col);
        frame = 0;
        text_mode = TEXT_SHOW;
    }
    else {
        frame++;
        if (frame == 180) {
            text_num++;
            text_mode = TEXT_LOAD;
            if (text_num == 2) {
                text_num = 0;
                print_init();
                return 1;
            }

        }
    }
    return 0;
}

int intro_run() {

    switch(state) {
        case STATE_INTRO_INIT:
            SCL_SetColMixRate(SCL_NBG0, 31);

            intro_init();
            sound_cdda(3, 1);
            cursor = 0;
            state = STATE_INTRO_LOADGIRL;
            break;
        
        case STATE_INTRO_LOADGIRL:
            SCL_SetColRam(SCL_NBG0, 0, 256, girl_pal[cursor]); //load palette
            SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, 0, 0, 0);
            // SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255);
            DMA_CpuMemCopy1(tile_ptr, girls[cursor], 256 * 17 * 30); //load tiles
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
                if (girl_xpos <= MTH_FIXED(-(SCREEN_X - GIRL_X))) {
                    girl_xpos = MTH_FIXED(-(SCREEN_X - GIRL_X));
                    state = STATE_INTRO_TEXT;
                }
            }
            else {
                girl_xpos += MOVE_SPEED;
                if (girl_xpos > MTH_FIXED(0)) {
                    girl_xpos = MTH_FIXED(0);
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
                    state = STATE_INTRO_END;
                }
                else {
                    state = STATE_INTRO_LOADGIRL;
                }
            }
            break;

        case STATE_INTRO_END:
            return 1;
            break;            
    }
    //move stars
    star_angle += MTH_FIXED(0.1);
    if (star_angle > MTH_FIXED(180)) {
        star_angle = MTH_FIXED(-180);
    }
    star_x += MTH_Mul(MTH_Cos(star_angle), MTH_FIXED(10));
    star_y += MTH_Mul(MTH_Sin(star_angle), MTH_FIXED(10));

    scroll_set(1, star_x, star_y);
    scroll_set(2, star_x >> 1, star_y >> 1);
    scroll_set(3, star_x >> 2, star_y >> 2);

    return 0;
}
