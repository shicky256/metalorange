#ifndef LASER_H
#define LASER_H

typedef struct {
    Fixed32 x;
    Fixed32 y;
    int index;
} LASER_SPRITE;

#define MAX_LASERS (16)

extern LASER_SPRITE laser_sprites[MAX_LASERS];
extern int laser_count;

void laser_init(int charno);
void laser_add(Fixed32 x, Fixed32 y);
void laser_remove(LASER_SPRITE *sprite);
void laser_move();

#endif
