#ifndef TILE_H
#define TILE_H

typedef enum {
	FRAMEBUFFER = 0,
	TILE_8X8,
	TILE_16x16,
} TILE_TYPE;

int tile_process(char *filename, char *outname, int type);

#endif
