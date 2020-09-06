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
    STATE_GAME_INIT = 0,
    STATE_GAME_FADEIN,
    STATE_GAME_HAND,
    STATE_GAME_PLAY
} GAME_STATE;

static int state = STATE_GAME_INIT;

#define CHIP_XPIXELS (96)
#define CHIP_YPIXELS (80)
#define CHIP_XTILES (CHIP_XPIXELS / 16)
#define CHIP_YTILES (CHIP_YPIXELS / 16)
#define IMAGE_XTILES (9)

#define CHIPFRAME_SIZE (96 * 80)
#define CHIP_HAND_FRAMES (4)
static int chip_hand_timings[CHIP_HAND_FRAMES] = {7, 6, 6, 35};
static int anim_cursor;

static inline void game_init() {
    Uint16 *game_buf = (Uint16 *)LWRAM;
    //blank display
    scroll_lores();
    SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2, -255, -255, -255);
    //wipe tilemaps
    scroll_clearmaps();
    //reset nbg0
    scroll_set(0, MTH_FIXED(0), MTH_FIXED(0));
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

int game_run() {
    static int frames;
    switch (state) {
        case STATE_GAME_INIT:
            game_init();
            frames = 0;
            state = STATE_GAME_FADEIN;
            break;
        
        case STATE_GAME_FADEIN:
            frames++;
            if (frames > 90) {
                anim_cursor = 0;
                frames = 0;
                game_loadchip(0);
                state = STATE_GAME_HAND;
            }
            break;
        
        case STATE_GAME_HAND: //chip does her hand thing here
            frames++;
            if (frames >= chip_hand_timings[anim_cursor]) {
                frames = 0;
                anim_cursor++;
                if (anim_cursor >= CHIP_HAND_FRAMES) {
                    state = STATE_GAME_PLAY;
                    break;
                }
                game_loadchip(anim_cursor);
            }
            break;
        
        case STATE_GAME_PLAY:
            if (PadData1E & PAD_A) {
                anim_cursor = 0;
                frames = 0;
                game_loadchip(0);
                state = STATE_GAME_HAND;
            }
            break;
    }
    return 0;
}
