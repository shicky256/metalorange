#ifndef SOUND_H
#define SOUND_H

#define SOUND_EXPLOSION (0)
#define SOUND_JUMP (1)
#define SOUND_DASH (2)
//must be called after cd_init
void sound_init(void);

#define CDDA_START (2)
typedef enum {
    LOGO_TRACK = CDDA_START,
    INTRO_TRACK,
    MENU_TRACK,
    PAUSE_TRACK,
    LEVEL1_TRACK,
    SOON_TRACK,
} CDDA_INDEX;

//play an audio track. loop: 1 if we want to loop the track
void sound_cdda(int track, int loop);
//play a pcm sound
void sound_play(short num);
#endif
