#include <sega_dma.h>
#include <sega_mth.h>
#include <sega_scl.h>
#include <string.h>

#include "cd.h"
#include "print.h"
#include "scroll.h"
#include "soon.h"
#include "sound.h"
#include "sprite.h"

typedef enum {
    STATE_SOON_INIT = 0,
    STATE_SOON_RUN,
} SOON_STATE;

// width in tiles of the "COMIN' SOON" graphic
#define SOON_WIDTH (44)
#define SOON_HEIGHT (8)
#define OFFSCREEN_TILE (SCROLL_LORES_X / 16)
// roughly centers the graphic
#define SOON_YSCROLL (MTH_FIXED(-40))

int soon_state = STATE_SOON_INIT;
#define SOON_MAXSCROLL (1140)
int soon_scroll;

void soon_init() {
	//wipe out vram
	memset((void *)SCL_VDP2_VRAM, 0, 0x80000);
    // clear out sprites
    sprite_deleteall();
    // low res
    scroll_lores();
    // fix vdp2 config from game
    scroll_charsize(0, SCL_CHAR_SIZE_2X2);
    scroll_mapsize(0, SCL_PN1WORD);
    char *img_buf = (char *)LWRAM;
    // load image
    scroll_set(1, MTH_FIXED(0), SOON_YSCROLL);
    cd_load("SOON.TLE", img_buf);
    scroll_loadtile(img_buf, (void *)SCL_VDP2_VRAM_B1, SCL_NBG1, 0);
    // play music
    sound_cdda(SOON_TRACK, 1);
    // reset scroll variable
    soon_scroll = 0;
}

inline void soon_loadcol(int column, int dest) {
    Uint16 *map_ptr = MAP_PTR(1);

    for (int i = 0; i < SOON_HEIGHT; i++) {
        map_ptr[(i * 32) + dest] = (i * SOON_WIDTH) + column;
    }
}

inline void soon_clearcol(int dest) {
    Uint16 *map_ptr = MAP_PTR(1);

    for (int i = 0; i < SOON_HEIGHT; i++) {
        map_ptr[(i * 32) + dest] = 0;
    }
}

int soon_run() {
    switch (soon_state){
        case STATE_SOON_INIT:
            soon_init();
            soon_state = STATE_SOON_RUN;
            break;

        case STATE_SOON_RUN:
            soon_scroll += 1;
            int source = soon_scroll / 16; // source column from image
            if (source < SOON_WIDTH) {
                soon_loadcol(source, (source + OFFSCREEN_TILE) % 32);   
            }
            else {
                soon_clearcol((source + OFFSCREEN_TILE) % 32);
            }
            scroll_set(1, MTH_IntToFixed(soon_scroll), SOON_YSCROLL);
            if (soon_scroll >= SOON_MAXSCROLL) {
                soon_state = STATE_SOON_INIT;
                return 1;
            }
            break;
    }

    return 0;
}
