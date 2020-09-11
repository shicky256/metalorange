#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>
#define _SPR2_
#include <SEGA_SPR.H>

#include "cd.h"
#include "game.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "vblank.h"

typedef enum {
    STATE_GAME_INIT = 0,
    STATE_GAME_FADEIN,
    STATE_GAME_PLAY
} GAME_STATE;

static int state = STATE_GAME_INIT;

typedef enum {
    STATE_CHIP_NONE = 0,
    STATE_CHIP_HAND,
    STATE_CHIP_SBLINK,
    STATE_CHIP_DBLINK
} CHIP_STATE;

static int chip_state = STATE_CHIP_NONE;
static int chip_frames = 0;
static int chip_cursor;

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
static int chip_blinktimer;

#define SHIP_CHARNO (font_num)
//start of ship's idle animation
#define SHIP_IDLE (SHIP_CHARNO + 13)
static SPRITE_INFO *ship_sprite;
//ship's y pos during normal gameplay
#define SHIP_POS (MTH_FIXED(200))
#define SHIP_SPEED (MTH_FIXED(4))
//all the ship's animations take 6 frames
#define SHIP_TIMING (6)
static int ship_frames;
//the area between where the sprite is and where the actual pixel graphics start
#define SHIP_MARGIN (MTH_FIXED(4))

//leftmost area of the playfield
#define LEFT_BOUND (MTH_FIXED(16) - SHIP_MARGIN)
#define RIGHT_BOUND (MTH_FIXED(224) - MTH_FIXED(ship_width) + SHIP_MARGIN)

static inline void game_init() {
    Uint8 *game_buf = (Uint8 *)LWRAM;
    //blank display
    scroll_lores();
    SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2 | SCL_NBG3, -255, -255, -255);
    //set sprite priority
    SCL_SetPriority(SCL_SPR,  7);
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
    Uint16 *map_buf = (Uint16 *)LWRAM;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (i < game_height && j < game_width) {
                game_map[i * 32 + j] = map_buf[i * game_width + j];
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

    //load sprite frames
    cd_load_nosize(ship_name, game_buf);
    SCL_SetColRam(SCL_SPR, 16, 16, ship_pal);
    SPR_2ClrAllChar();
    for (int i = 0; i < ship_num; i++) {
        // SPR_2ClrChar(i + SHIP_CHARNO);
        SPR_2SetChar(i + SHIP_CHARNO, COLOR_0, 16, ship_width, ship_height, (char *)game_buf + (i * ship_size));
    }
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

static void game_chipset(int state) {
    chip_cursor = 0;
    chip_frames = 0;
    chip_state = state;
    switch(state) {
        case STATE_CHIP_NONE:
            game_loadchip(0);
            break;

        case STATE_CHIP_HAND:
            game_loadchip(chip_hand_frames[0]);
            break;

        case STATE_CHIP_SBLINK:
        case STATE_CHIP_DBLINK:
            game_loadchip(chip_blink_frames[0]);
            break;
    }
}

static void game_chipanim() {
    switch (chip_state) {
        case STATE_CHIP_HAND: //chip does her hand thing here
            chip_frames++;
            if (chip_frames >= chip_hand_timings[chip_cursor]) {
                chip_frames = 0;
                chip_cursor++;
                if (chip_cursor >= CHIP_HAND_NUMFRAMES) {
                    game_chipset(STATE_CHIP_NONE);
                    break;
                }
                game_loadchip(chip_hand_frames[chip_cursor]);
            }
            break;

        case STATE_CHIP_SBLINK:
        case STATE_CHIP_DBLINK:
            chip_frames++;
            if (chip_frames >= chip_blink_timings[chip_cursor]) {
                chip_frames = 0;
                chip_cursor++;
                if (chip_cursor >= CHIP_BLINK_NUMFRAMES) {
                    if (chip_state == STATE_CHIP_DBLINK) {
                        game_chipset(STATE_CHIP_SBLINK);
                    }
                    else {
                        game_chipset(STATE_CHIP_NONE);
                    }
                    break;
                }
                game_loadchip(chip_blink_frames[chip_cursor]);
            }
            break;
    }
}

static inline int game_setblinktimer() {
    return 180 + (MTH_GetRand() & 0x1f);
}

static inline void game_shipanim() {
    //-----ship animation stuff (keeping it stateless helps simplify things)
    //move ship when it spawns
    if (ship_sprite->y > SHIP_POS) {
        ship_sprite->y -= MTH_FIXED(1.5);
    }
    else {
        ship_sprite->y = SHIP_POS;
    }
    //animate ship
    ship_frames++;
    if (ship_frames == SHIP_TIMING) {
        ship_frames = 0;
        ship_sprite->char_num++;
        if (ship_sprite->char_num == SHIP_CHARNO + ship_num) {
            ship_sprite->char_num = SHIP_IDLE;
        }
    }
}

int game_run() {
    static int frames;
    switch (state) {
        case STATE_GAME_INIT:
            game_init();
            game_chipset(STATE_CHIP_NONE);
            chip_blinktimer = game_setblinktimer();
            frames = 0;
            //init the ship sprite
            ship_sprite = sprite_next();
            sprite_make(SHIP_CHARNO, MTH_FIXED(102), MTH_FIXED(240), ship_sprite);
            ship_frames = 0;

            state = STATE_GAME_FADEIN;
            break;
        
        case STATE_GAME_FADEIN:
            frames++;
            if (frames > 90) {
                game_chipset(STATE_CHIP_HAND);
                frames = 0;
                state = STATE_GAME_PLAY;
            }
            break;
        
        case STATE_GAME_PLAY:
            //handle chip animation stuff
            frames++;
            if (frames == chip_blinktimer) {
                frames = 0;
                chip_blinktimer = game_setblinktimer();
                //chip blinks twice 1/4 of the time
                if (((MTH_GetRand() >> 8) & 0xf) < 4) {
                    game_chipset(STATE_CHIP_DBLINK);
                }
                else {
                    game_chipset(STATE_CHIP_SBLINK);
                }
            }
            //handle input
            if (PadData1 & PAD_L) {
                ship_sprite->x -= SHIP_SPEED;
            }
            if (PadData1 & PAD_R) {
                ship_sprite->x += SHIP_SPEED;
            }
            //ship boundaries
            if (ship_sprite->x < LEFT_BOUND) {
                ship_sprite->x = LEFT_BOUND;
            }
            if (ship_sprite->x > RIGHT_BOUND) {
                ship_sprite->x = RIGHT_BOUND;
            }
            break;
    }

    game_chipanim();
    game_shipanim();

    scroll_move(1, MTH_FIXED(0), MTH_FIXED(-4));
    scroll_move(2, MTH_FIXED(0), MTH_FIXED(-2));
    scroll_move(3, MTH_FIXED(0), MTH_FIXED(-1));
    return 0;
}
