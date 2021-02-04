#include <sega_dma.h>
#include <sega_mth.h>
#include <sega_scl.h>

#include "cd.h"
#include "scroll.h"
#include "sound.h"
#include "vblank.h"

typedef enum {
    CUTSCENE_INIT = 0,
    CUTSCENE_RUN,
    CUTSCENE_DONEPRINT,
    CUTSCENE_FADEOUT,
} CUTSCENE_STATE;

static int state = CUTSCENE_INIT;
static int frames;
static int print_x, print_y, print_cursor;

char *face_ptr;
// dimensions of a single eye frame
#define EYE_WIDTH (128)
#define EYE_HEIGHT (94)

// where the eye frame goes on the screen
#define EYE_XPOS (296)
#define EYE_YPOS (90)

#define MOUTH_OFFSET (282 * EYE_WIDTH)
static int mouth_frame;
// dimensions of a single mouth frame
#define MOUTH_WIDTH (32)
#define MOUTH_HEIGHT (24)

// where the mouth frame goes on the screen
#define MOUTH_XPOS (323)
#define MOUTH_YPOS (183)

char *font_ptr;

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
                
// number of frames between printing characters
#define CHAR_FRAMES (6)

#define EYE_COUNT (4)
int eye_frames[EYE_COUNT] = {0, 1, 2, 1};
int eye_timings[EYE_COUNT] = {6, 4, 6, 4};
int eye_cursor = EYE_COUNT;
int eye_timer = 0;

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

    // load graphic
    cd_load("TOMO.TLE", gfx_ptr);
    scroll_loadtile(gfx_ptr, NULL, SCL_NBG0, 0);
    gfx_ptr = scroll_tileptr(gfx_ptr, NULL);
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            vram[BMP_WIDTH * y + x] = gfx_ptr[IMG_WIDTH * y + x];
        }
    }

    gfx_ptr = (char *)LWRAM;
    int frame_size = cd_load("TOMOFACE.TLE", gfx_ptr);
    face_ptr = scroll_tileptr(gfx_ptr, NULL);
    

    // load font
    font_ptr = (char *)(LWRAM + frame_size);
    cd_load("CUTSCFON.TLE", font_ptr);
    font_ptr = scroll_tileptr(font_ptr, NULL);

    // set up auto fade-in
    SclRgb start, end;
    start.red = start.green = start.blue = -255;
    end.red = end.green = end.blue = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
    // set up music
    sound_cdda(TOMO_TRACK, 1);
}

static void cutscene_eyes(int frame) {
    char *eyeframe_ptr = face_ptr + (frame * EYE_WIDTH * EYE_HEIGHT);
    char *vram = (char *)SCL_VDP2_VRAM;
    for (int y = 0; y < EYE_HEIGHT; y++) {
        for (int x = 0; x < EYE_WIDTH; x++) {
            vram[(y + EYE_YPOS) * BMP_WIDTH + (x + EYE_XPOS)] = eyeframe_ptr[y * EYE_WIDTH + x];
        }
    }
}

static void cutscene_mouth(int frame) {
    char *mouthframe_ptr = face_ptr + MOUTH_OFFSET;
    char *vram = (char *)SCL_VDP2_VRAM;
    for (int y = 0; y < MOUTH_HEIGHT; y++) {
        for (int x = 0; x < MOUTH_WIDTH; x++) {
            vram[(y + MOUTH_YPOS) * BMP_WIDTH + (x + MOUTH_XPOS)] = mouthframe_ptr[(frame * MOUTH_WIDTH) + (y * EYE_WIDTH) + x];
        }
    }
}

static int cutscene_print(int x_base, int *x_pos, int *y_pos, char *str, int *cursor) {
    char *vram = (char *)SCL_VDP2_VRAM;
    str += *cursor;
    char ch;
    int retval = 0;
    while (1) {
        ch = *str;
        switch (ch) {
            // if it's the end of a string, exit
            case '\0':
                return 2;

            case '\n':
                *y_pos += CHAR_HEIGHT;
                *x_pos = x_base;
                (*cursor)++;
                str++;
                retval = 1;
                continue;

            default:
                break;
        }
        // break out of the loop if ch isn't a null terminator or a newline
        break;
    }
    (*cursor)++;
    ch -= 32; // font starts with the space character
    // each row in the font image has 16 characters
    // pointer to top left pixel of the character in the font
    char *letter_ptr = font_ptr + ((ch >> 4) * (CHAR_HEIGHT * FONT_WIDTH)) + ((ch & 0xf) * CHAR_WIDTH);
    for (int y = 0; y < CHAR_HEIGHT; y++) {
        for (int x = 0; x < CHAR_WIDTH; x++) {
            vram[(y + *y_pos) * BMP_WIDTH + (x + *x_pos)] = letter_ptr[y * FONT_WIDTH + x];
        }
    }
    *x_pos += CHAR_WIDTH;
    return retval;
}

int cutscene_run() {
    // blink animation
    if (eye_cursor < EYE_COUNT) {
        cutscene_eyes(eye_frames[eye_cursor]);
        eye_timer++;
        if (eye_timer >= eye_timings[eye_cursor]) {
            eye_timer = 0;
            eye_cursor++;
        }
    }

    else {
        cutscene_eyes(0);
    }

    switch (state) {
        case CUTSCENE_INIT:
            cutscene_init();
            print_x = 120;
            print_y = 350;
            print_cursor = 0;
            mouth_frame = 0;
            cutscene_print(120, &print_x, &print_y, tomo_text, &print_cursor);
            cutscene_mouth(1);
            state = CUTSCENE_RUN;
            break;
        
        case CUTSCENE_RUN:
            frames++;
            if (frames == CHAR_FRAMES) {
                frames = 0;
                mouth_frame ^= 1;
                cutscene_mouth(mouth_frame);
                switch (cutscene_print(120, &print_x, &print_y, tomo_text, &print_cursor)) {
                    case 0:
                        break;
                    
                    case 1: // blink at newlines
                        eye_cursor = 0;
                        break;
                    
                    case 2: // close mouth and random blink at end of text 
                        cutscene_mouth(0);
                        state = CUTSCENE_DONEPRINT;
                        break;
                }
            }
            break;

        // when cutscene is done printing, make tomo blink at random intervals
        case CUTSCENE_DONEPRINT:
            if (frames == 0) {
                eye_cursor = 0;
                frames = ((MTH_GetRand() >> 16) & 0x1ff) + 180;
            }
            frames--;
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

    // fade out on button press
    if (PadData1E && (state >= CUTSCENE_RUN)) {
        frames = 0;
        SclRgb start, end;
        start.red = start.green = start.blue = 0;
        end.red = end.green = end.blue = -255;
        SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
        state = CUTSCENE_FADEOUT;
    }

    return 0;
}
