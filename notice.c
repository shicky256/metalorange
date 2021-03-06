#include <sega_dma.h>
#include <sega_scl.h>
#include <sega_xpt.h>
#include <string.h>

#include "cd.h"
#include "devcart.h"
#include "notice.h"
#include "print.h"
#include "scroll.h"
#include "vblank.h"

#define NOTICE_X (1)
#define NOTICE_Y (2)

int notice_displayed = 0;
char *notice_text = "\"Cyberblock Metal Orange\" for Sega Saturn\n"
                    "    http://www.infochunk.com/orange/\n"
                    "------------------------------------------\n\n"
                    "Art, Sounds, Music (c) 1993 CUSTOM\n"
                    "Saturn sound engine (c) 2020 Ponut64\n"
                    "Saturn program written from scratch by\n"
                    "ndiddy. Please excuse any inaccuracies.\n\n"
                    "Only the first couple levels are included,\n"
                    "I may make a full version at a later date.\n\n"
                    "This is a noncommercial hobby project.\n"
                    "Please do not sell copies of this game.\n\n"
                    "Due to the original game being meant for\n"
                    "computers, you may have to adjust your TV\n"
                    "settings in order to view the entire\n"
                    "playfield.\n\n"
                    "Special thanks:\n"
                    "fafling        mrkotfw\n"
                    "Ponut64        vbt\n\n"
                    "Press any button to continue.";

void notice_print(int x, int y, char *str) {
    int x_bak = x;
    char ch;
    Uint16 *map_ptr = MAP_PTR(1);

    while ((ch = *str) != '\0') {
        if (ch == '\n') {
            x = x_bak;
            y++;
        }
        else {
            map_ptr[(y * 64) + x] = (Uint16)(ch - 32);
            x++;
        }
        str++;
    }
}

int notice_run() {
    if (notice_displayed == 0) {
        scroll_clearmaps();
        scroll_lores();
        scroll_charsize(1, SCL_CHAR_SIZE_1X1);
        scroll_set(1, 0, 0);
        scroll_enable(2, OFF);
        scroll_enable(3, OFF);
        cd_load("NOTICEFO.TLE", (void *)LWRAM);
        scroll_loadtile((void *)LWRAM, (void *)SCL_VDP2_VRAM_B1, SCL_NBG1, 0);
        notice_print(NOTICE_X, NOTICE_Y, notice_text);
        notice_displayed = 1;
    }

    if (PadData1E) {
        notice_displayed = 0;
        scroll_charsize(1, SCL_CHAR_SIZE_2X2);
        // devcart_printstr("scroll_charsize\n");
        // scroll_clearmaps();
        memset((void *)SCL_VDP2_VRAM, 0, 0x80000);
        scroll_enable(2, ON);
        scroll_enable(3, ON);
        return 1;
    }
    return 0;
}
