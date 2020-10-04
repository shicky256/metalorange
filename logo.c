#include <SEGA_DMA.H>
#include <SEGA_SCL.H>

#include "cd.h"
#include "graphicrefs.h"
#include "scroll.h"
#include "sound.h"
#include "vblank.h"
#include "logo.h"

#define FRAME_XPIXELS (144)
#define FRAME_YPIXELS (128)
#define FRAME_XTILES (9)
#define FRAME_YTILES (8)
#define IMAGE_XTILES (18)

typedef enum {
	STATE_LOGO_INIT = 0,
	STATE_LOGO_FADEIN,
	STATE_LOGO_RUN,
	STATE_LOGO_DONE,
	STATE_LOGO_FADEOUT,
	STATE_LOGO_END
} LOGO_STATE;

static int timings[] = {186, 10, 10, 8, 66, 16, 8, 52, 9, 12, 16, 18, 39, 12, 5};
static int timer = 0;
static int cursor = 0;
static int state = STATE_LOGO_INIT;

void logo_loadframe(int num) {
    char *frame_ptr = ((char *)LWRAM) + (num * (FRAME_XPIXELS * FRAME_YPIXELS));
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1 + (256 * IMAGE_XTILES) + (256 * 5); //start at one row + 5 tiles in

    int offset;
    for (int i = 0; i < FRAME_YTILES; i++) {
        offset = i * (256 * IMAGE_XTILES);
        for (int j = 0; j < 256 * FRAME_XTILES; j++) {
            tile_ptr[offset++] = frame_ptr[j + (i * 256 * FRAME_XTILES)];
        }
    }
}

static inline void logo_init() {
    char *logo_buf = (char *)LWRAM;
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1;
    Uint16 *map_ptr = MAP_PTR(0);
    //put the image in a good location
    scroll_set(0, MTH_FIXED(-208), MTH_FIXED(-80));
    cd_load_nosize(roarbg_name, logo_buf);
    DMA_CpuMemCopy1(tile_ptr, logo_buf, 256 * roarbg_num);
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
    cd_load_nosize(roarframes_name, logo_buf);
}

int logo_run(void) {
    static int frame;

    //allow user to skip the logo
    if ((PadData1 & PAD_S) && state > STATE_LOGO_FADEIN) {
        SclRgb start, end;
        start.red = start.green = start.blue = 0;
        end.red = end.green = end.blue = -255;
        SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 60, &start, &end);
        frame = 0;
        state = STATE_LOGO_FADEOUT;
    }

    switch(state) {
        case STATE_LOGO_INIT:
            //turn brightness all the way down
            SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255);
            SCL_DisplayFrame();
            logo_init();
            SclRgb start, end;
            start.red = start.green = start.blue = -255;
            end.red = end.green = end.blue = 0;
            SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 60, &start, &end);
            frame = 0;
            state = STATE_LOGO_FADEIN;
            break;
        
        case STATE_LOGO_FADEIN:
            frame++;
            if (frame > 60) {
                sound_cdda(2, 0); //play logo song
                state = STATE_LOGO_RUN;
            }
            break;

        case STATE_LOGO_RUN:
            if (cursor < (sizeof(timings) / sizeof(timings[0]))) {
                timer++;
                if (timer > timings[cursor]) {
                    logo_loadframe(cursor++);
                    timer = 0;
                    if (cursor == 9) {
                        //play roar sound
                        sound_play(0);
                    }
                }
            }
            else {
                frame = 0;
                state = STATE_LOGO_DONE;
            }
            break;

        case STATE_LOGO_DONE:
            frame++;
            if (frame > 60) {
                SclRgb start, end;
                start.red = start.green = start.blue = 0;
                end.red = end.green = end.blue = -255;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 60, &start, &end);
                frame = 0;
                state = STATE_LOGO_FADEOUT;
            }
            break;
            
        case STATE_LOGO_FADEOUT:
            frame++;
            if (frame > 60) {
                state = STATE_LOGO_END;
            }
            break;

        case STATE_LOGO_END:
            return 1;
            break;
    }
    return 0;
}
