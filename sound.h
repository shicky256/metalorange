#ifndef SOUND_H
#define SOUND_H

//must be called after cd_init
void sound_init(void);

#define CDDA_START (2)
typedef enum {
    LOGO_TRACK = CDDA_START,
    INTRO_TRACK,
    MENU_TRACK,
    PAUSE_TRACK,
    TOMO_TRACK,
    LEVEL1_TRACK,
    SOON_TRACK,
} CDDA_INDEX;

//play an audio track. loop: 1 if we want to loop the track
void sound_cdda(int track, int loop);

typedef enum {
    SOUND_ROAR = 0,
    SOUND_SELECT,
    SOUND_START,
    SOUND_BLOCK,
    SOUND_DEATH,
    SOUND_SHIP,
    SOUND_GOLD,
    SOUND_CAPSULE,
    SOUND_POWERUP,
    SOUND_LASER,
    SOUND_REPLACE,
} PCM_INDEX;

//play a pcm sound
void sound_play(short num);
#endif
