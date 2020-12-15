#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>

#include "cd.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "vblank.h"

typedef enum {
    CUTSCENE_INIT = 0,
    CUTSCENE_RUN,
    CUTSCENE_FADEOUT,
} CUTSCENE_STATE;

static int state = CUTSCENE_INIT;
static int frames;

// dimensions of the image that's copied to vram
#define IMG_WIDTH (SCROLL_HIRES_X)
#define IMG_HEIGHT (SCROLL_HIRES_Y)
// dimensions of the vdp2 framebuffer
#define BMP_WIDTH (1024)
#define BMP_HEIGHT (512)

static inline void cutscene_init() {
    char *gfx_ptr = (char *)LWRAM;
    char *vram = (char *)SCL_VDP2_VRAM;
    // hires gfx + enable bitmap
    scroll_hires();
    scroll_bitmapon();
    // black out screen
    SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255);
    // reset scroll
    scroll_set(0, 0, 0);

    // load palette
    SCL_SetColRam(SCL_NBG0, 0, 256, tomo_pal);

    // load graphic
    cd_load_nosize(tomo_name, gfx_ptr);
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            vram[BMP_WIDTH * y + x] = gfx_ptr[IMG_WIDTH * y + x];
        }
    }
    // set up auto fade-in
    SclRgb start, end;
    start.red = start.green = start.blue = -255;
    end.red = end.green = end.blue = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
    // set up music
    sound_cdda(TOMO_TRACK, 1);
}

int cutscene_run() {
    switch (state) {
        case CUTSCENE_INIT:
            cutscene_init();
            state = CUTSCENE_RUN;
            break;
        
        case CUTSCENE_RUN:
            // fade out on button press
            if (PadData1E) {
                frames = 0;
                SclRgb start, end;
                start.red = start.green = start.blue = 0;
                end.red = end.green = end.blue = -255;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
                state = CUTSCENE_FADEOUT;
            }
            break;
        
        case CUTSCENE_FADEOUT:
            frames++;
            if (frames >= 30) {
                scroll_bitmapoff();
                state = CUTSCENE_INIT;
                return 1;
            }
            break;
    }
    return 0;
}
