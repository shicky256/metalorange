#include <SEGA_CDC.H>
#include <SEGA_INT.H>

#include "cd.h"
#include "sound.h"
#include "pcmsys.h"
#include "print.h"

static void sound_external_audio_enable(Uint8 vol_l, Uint8 vol_r) {
    volatile Uint16 *slot_ptr;

    //max sound volume is 7
    if (vol_l > 7) {
        vol_l = 7;
    }
    if (vol_r > 7) {
        vol_r = 7;
    }

    // Setup SCSP Slot 16 and Slot 17 for playing
    slot_ptr = (volatile Uint16 *)(0x25B00000 + (0x20 * 16));
    slot_ptr[0] = 0x1000;
    slot_ptr[1] = 0x0000; 
    slot_ptr[2] = 0x0000; 
    slot_ptr[3] = 0x0000; 
    slot_ptr[4] = 0x0000; 
    slot_ptr[5] = 0x0000; 
    slot_ptr[6] = 0x00FF; 
    slot_ptr[7] = 0x0000; 
    slot_ptr[8] = 0x0000; 
    slot_ptr[9] = 0x0000; 
    slot_ptr[10] = 0x0000; 
    slot_ptr[11] = 0x001F | (vol_l << 5);
    slot_ptr[12] = 0x0000; 
    slot_ptr[13] = 0x0000; 
    slot_ptr[14] = 0x0000; 
    slot_ptr[15] = 0x0000; 

    slot_ptr = (volatile Uint16 *)(0x25B00000 + (0x20 * 17));
    slot_ptr[0] = 0x1000;
    slot_ptr[1] = 0x0000; 
    slot_ptr[2] = 0x0000; 
    slot_ptr[3] = 0x0000; 
    slot_ptr[4] = 0x0000; 
    slot_ptr[5] = 0x0000; 
    slot_ptr[6] = 0x00FF; 
    slot_ptr[7] = 0x0000; 
    slot_ptr[8] = 0x0000; 
    slot_ptr[9] = 0x0000; 
    slot_ptr[10] = 0x0000; 
    slot_ptr[11] = 0x000F | (vol_r << 5);
    slot_ptr[12] = 0x0000; 
    slot_ptr[13] = 0x0000; 
    slot_ptr[14] = 0x0000; 
    slot_ptr[15] = 0x0000;

    *((volatile Uint16 *)(0x25B00400)) = 0x020F;
}

//must be called after cd_init
void sound_init() {
    sound_external_audio_enable(5, 5);
    load_drv();
    load_8bit_pcm("ROAR.RAW", 11025);
}

void sound_cdda(int track) {
    CdcPly ply;
    CDC_PLY_STYPE(&ply) = CDC_PTYPE_TNO; //track number
	CDC_PLY_STNO(&ply)  = track;
	CDC_PLY_SIDX(&ply) = 1;
	CDC_PLY_ETYPE(&ply) = CDC_PTYPE_TNO;
	CDC_PLY_ETNO(&ply)  = track + 1;
	CDC_PLY_EIDX(&ply) = 99;
	CDC_PLY_PMODE(&ply) = CDC_PM_DFL; // | 0xf; //0xf = infinite repetitions
    CDC_CdPlay(&ply);
}

void sound_play(short num) {
    pcm_play(num, PCM_SEMI, 6);
}

