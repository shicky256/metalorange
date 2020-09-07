#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>

#include "cd.h"
// #include "game.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "vblank.h"

typedef enum {
    STATE_ANIM_NONE = 0,
    STATE_ANIM_HAND,
    STATE_ANIM_SBLINK,
    STATE_ANIM_DBLINK
} ANIM_STATE;

static int anim_state = STATE_ANIM_NONE;
static int anim_frames = 0;

typedef enum {
    STATE_GAME_INIT = 0,
    STATE_GAME_FADEIN,
    STATE_GAME_PLAY
} GAME_STATE;

static int state = STATE_GAME_INIT;

#define CHIP_XPIXELS (96)
#define CHIP_YPIXELS (80)
#define CHIP_XTILES (CHIP_XPIXELS / 16)
#define CHIP_YTILES (CHIP_YPIXELS / 16)
#define IMAGE_XTILES (9)

#define CHIPFRAME_SIZE (96 * 80)
#define CHIP_HAND_NUMFRAMES (4)
static int chip_hand_frames[CHIP_HAND_NUMFRAMES] = {1, 2, 3, 4};
static int chip_hand_timings[CHIP_HAND_NUMFRAMES] = {7, 6, 6, 35};

#define CHIP_BLINK_NUMFRAMES (3)
static int chip_blink_frames[CHIP_BLINK_NUMFRAMES] = {5, 6, 0};
static int chip_blink_timings[CHIP_BLINK_NUMFRAMES] = {11, 4, 4};
static int anim_cursor;
static int chip_blinktimer;

static inline void game_init() {
    Uint16 *game_buf = (Uint16 *)LWRAM;
    //blank display
    scroll_lores();
    SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2 | SCL_NBG3, -255, -255, -255);
    //wipe tilemaps
    scroll_clearmaps();
    //reset nbg0
    scroll_set(0, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(1, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(2, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(3, MTH_FIXED(0), MTH_FIXED(0));
    //load tiles for hud
    cd_load_nosize(game_tiles_name, game_buf);
    //write them to the screen
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1;
    DMA_CpuMemCopy1(tile_ptr, game_buf, game_tiles_num * 256);
    //load palette for hud
    SCL_SetColRam(SCL_NBG0, 0, 256, game_tiles_pal);
    //load map
    cd_load_nosize(game_name, game_buf);
    Uint16 *game_map = MAP_PTR(0);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (i < game_height && j < game_width) {
                game_map[i * 32 + j] = game_buf[i * game_width + j];
            } 
            else {
                game_map[i * 32 + j] = 0;
            }
        }
    }
    //load tiles for stars
    cd_load_nosize(gamestar_name, game_buf);
    //write them to screen
    tile_ptr = (char *)SCL_VDP2_VRAM_B1;
    DMA_CpuMemCopy1(tile_ptr, game_buf, gamestar_num * 128);
    //load palette for stars
    SCL_SetColRam(SCL_NBG1, 0, 16, gamestar_pal);
    //load maps
    cd_load_nosize(gamenear_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(1), game_buf, 32 * 32 * 2);
    cd_load_nosize(gamemid_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(2), game_buf, 32 * 32 * 2);
    cd_load_nosize(gamefar_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(3), game_buf, 32 * 32 * 2);
    //fade in
    SclRgb start, end;
    start.red = start.green = start.blue = -255;
    end.red = end.green = end.blue = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);

    //load chip's animation frames into ram
    cd_load_nosize(chipgame_name, game_buf);
    sound_cdda(5, 1);
}

static void game_loadchip(int num) {
    char *frame_ptr = ((char *)LWRAM) + (num * (CHIP_XPIXELS * CHIP_YPIXELS));
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1 + ((256 * IMAGE_XTILES) * 2) + (256 * 2); //start at 3 rows + 3 tiles in

    int offset;
    for (int i = 0; i < CHIP_YTILES; i++) {
        offset = i * (256 * IMAGE_XTILES);
        for (int j = 0; j < 256 * CHIP_XTILES; j++) {
            tile_ptr[offset++] = frame_ptr[j + (i * 256 * CHIP_XTILES)];
        }
    }
}

static void game_animset(int state) {
    anim_cursor = 0;
    anim_frames = 0;
    anim_state = state;
    switch(state) {
        case STATE_ANIM_NONE:
            game_loadchip(0);
            break;

        case STATE_ANIM_HAND:
            game_loadchip(chip_hand_frames[0]);
            break;

        case STATE_ANIM_SBLINK:
        case STATE_ANIM_DBLINK:
            game_loadchip(chip_blink_frames[0]);
            break;
    }
}

static void game_animate() {
    switch (anim_state) {
        case STATE_ANIM_HAND: //chip does her hand thing here
            anim_frames++;
            if (anim_frames >= chip_hand_timings[anim_cursor]) {
                anim_frames = 0;
                anim_cursor++;
                if (anim_cursor >= CHIP_HAND_NUMFRAMES) {
                    game_animset(STATE_ANIM_NONE);
                    break;
                }
                game_loadchip(chip_hand_frames[anim_cursor]);
            }
            break;

        case STATE_ANIM_SBLINK:
        case STATE_ANIM_DBLINK:
            anim_frames++;
            if (anim_frames >= chip_blink_timings[anim_cursor]) {
                anim_frames = 0;
                anim_cursor++;
                if (anim_cursor >= CHIP_BLINK_NUMFRAMES) {
                    if (anim_state == STATE_ANIM_DBLINK) {
                        game_animset(STATE_ANIM_SBLINK);
                    }
                    else {
                        game_animset(STATE_ANIM_NONE);
                    }
                    break;
                }
                game_loadchip(chip_blink_frames[anim_cursor]);
            }
            break;
    }
}

static inline int game_setblinktimer() {
    return 180 + (MTH_GetRand() & 0x1f);
}

int game_run() {
    static int frames;
    switch (state) {
        case STATE_GAME_INIT:
            game_init();
            game_animset(STATE_ANIM_NONE);
            chip_blinktimer = game_setblinktimer();
            frames = 0;
            state = STATE_GAME_FADEIN;
            break;
        
        case STATE_GAME_FADEIN:
            frames++;
            if (frames > 90) {
                game_animset(STATE_ANIM_HAND);
                frames = 0;
                state = STATE_GAME_PLAY;
            }
            break;
        
        case STATE_GAME_PLAY:
            frames++;
            if (frames == chip_blinktimer) {
                frames = 0;
                chip_blinktimer = game_setblinktimer();
                //chip blinks twice 1/4 of the time
                if ((MTH_GetRand() & 0xf) < 4) {
                    game_animset(STATE_ANIM_DBLINK);
                }
                else {
                    game_animset(STATE_ANIM_SBLINK);
                }
            }
            break;
    }

    game_animate();
    scroll_move(1, MTH_FIXED(0), MTH_FIXED(-4));
    scroll_move(2, MTH_FIXED(0), MTH_FIXED(-2));
    scroll_move(3, MTH_FIXED(0), MTH_FIXED(-1));
    return 0;
}
