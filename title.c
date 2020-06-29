#include <SEGA_DMA.H>
#include <SEGA_SCL.H>

#include "cd.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "title.h"

#define FRAME_XPIXELS (144)
#define FRAME_YPIXELS (128)
#define FRAME_XTILES (9)
#define FRAME_YTILES (8)
#define IMAGE_XTILES (18)

static int timings[] = {186, 10, 10, 8, 66, 16, 8, 52, 9, 12, 16, 18, 39, 12, 5};
static int timer = 0;
static int cursor = 0;

void title_loadframe(int num) {
    char *frame_ptr = ((char *)LWRAM) + (num * (FRAME_XPIXELS * FRAME_YPIXELS));
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1 + (256 * IMAGE_XTILES) + (256 * 5);

    int offset;
    for (int i = 0; i < FRAME_YTILES; i++) {
        offset = i * (256 * IMAGE_XTILES);
        for (int j = 0; j < 256 * FRAME_XTILES; j++) {
            tile_ptr[offset++] = frame_ptr[j + (i * 256 * FRAME_XTILES)];
        }
    }
}

void title_init() {
    char *title_buf = (char *)LWRAM;
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1;
    Uint16 *map_ptr = VRAM_PTR(0);

    cd_load_nosize(roarbg_name, title_buf);
    for (int i = 0; i < 256 * roarbg_num; i++) {
        tile_ptr[i] = title_buf[i];
    }
    SCL_AllocColRam(SCL_NBG0, 256, OFF);
    SCL_SetColRam(SCL_NBG0, 0, 256, roarbg_pal);

    int xnum = 0;
    int ynum = 0;

    for (int i = 0; i < roarbg_num; i++) {
        map_ptr[ynum * 32 + xnum] = i * 2;
        xnum++;
        if (xnum == 18) {
            xnum = 0;
            ynum++;
        }
    }

    //load rest of frames into LWRAM to prepare for loading to VRAM
    cd_load_nosize(roarframes_name, title_buf);
}

void title_run(void) {
    if (cursor < (sizeof(timings) / sizeof(timings[0]))) {
        timer++;
        if (timer > timings[cursor]) {
            title_loadframe(cursor++);
            timer = 0;
            if (cursor == 9) {
                sound_play(0);
            }
        }
    }
}
