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
// dimensions of font graphic
#define FONT_WIDTH (256)
#define CHAR_WIDTH (16)
#define CHAR_HEIGHT (32)

char tomo_text[] = "Hi, I'm Tomo. Only a couple\n"
                   "of my levels are programmed in\n"
                   "right now. Oh well...";

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

    // load font
    cd_load_nosize(cutscfont_name, gfx_ptr);
    // set up auto fade-in
    SclRgb start, end;
    start.red = start.green = start.blue = -255;
    end.red = end.green = end.blue = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
    // set up music
    sound_cdda(TOMO_TRACK, 1);
}

static void cutscene_print(int x_pos, int y_pos, char *str) {
    int x_bak = x_pos;
    char *vram = (char *)SCL_VDP2_VRAM;
    char ch;
    while ((ch = *str) != '\0') {
        if (ch == '\n') {
            y_pos += CHAR_HEIGHT;
            x_pos = x_bak;
            str++;
            continue;
        }

        ch -= 32; // font starts with the space character
        // each row in the font image has 16 characters
        int graphic_start = LWRAM + ((ch >> 4) * (CHAR_HEIGHT * FONT_WIDTH)) + ((ch & 0xf) * CHAR_WIDTH);
        // pointer to top left pixel of the character in the font
        char *graphic_ptr = (char *)graphic_start;
        for (int y = 0; y < CHAR_HEIGHT; y++) {
            for (int x = 0; x < CHAR_WIDTH; x++) {
                vram[(y + y_pos) * BMP_WIDTH + (x + x_pos)] = graphic_ptr[y * FONT_WIDTH + x];
            }
        }
        x_pos += CHAR_WIDTH;
        str++;
    }
}

int cutscene_run() {
    switch (state) {
        case CUTSCENE_INIT:
            cutscene_init();
            cutscene_print(120, 350, tomo_text);
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
