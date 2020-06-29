#ifndef SOUND_H
#define SOUND_H

#define SOUND_EXPLOSION (0)
#define SOUND_JUMP (1)
#define SOUND_DASH (2)
//must be called after cd_init
void sound_init(void);
//play an audio track
void sound_cdda(int track);
//play a pcm sound
void sound_play(short num);
#endif
