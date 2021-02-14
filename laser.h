#ifndef LASER_H
#define LASER_H
#include "sprite.h"

typedef struct {
    SPRITE_INFO *next;
    SPRITE_INFO *prev;
} LASER_DATA;

#define MAX_LASERS (16)

extern SPRITE_INFO *laser_head;
extern int laser_count;

void laser_init(int charno);
void laser_add(Fixed32 x, Fixed32 y);
void laser_remove(SPRITE_INFO *sprite);
void laser_removeall();

#endif
