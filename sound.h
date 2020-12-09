#ifndef SOUND_H
#define SOUND_H

#define SOUND_EXPLOSION (0)
#define SOUND_JUMP (1)
#define SOUND_DASH (2)
//must be called after cd_init
void sound_init(void);

#define LOGO_TRACK (2)
#define INTRO_TRACK (3)
#define MENU_TRACK (4)
#define PAUSE_TRACK (5)
#define LEVEL1_TRACK (6)
//play an audio track. loop: 1 if we want to loop the track
void sound_cdda(int track, int loop);
//play a pcm sound
void sound_play(short num);
#endif
