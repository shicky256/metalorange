#ifndef SCROLL_H
#define SCROLL_H

#define MAP_PTR(bg) ((Uint16 *)vram[bg])
extern Uint32 vram[];

#define SCROLL_HIRES_X (704)
#define SCROLL_HIRES_Y (480)
#define SCROLL_LORES_X (352)
#define SCROLL_LORES_Y (240)

#define SCROLL_RES_LOW (0)
#define SCROLL_RES_HIGH (1)
extern int scroll_res;

typedef struct {
    Uint8 *tile_name; //filename of .TLE file
    Uint16 tile_num; //number of tiles
    Uint32 *palette; //pointer to palette
    Uint8 *map_name; //filename of .MAP file
    Uint16 map_width; //width of map
    Uint16 map_height; //height of map
} LAYER;

void scroll_init(void);
void scroll_lores(void);
void scroll_hires(void);
void scroll_scale(int num, Fixed32 scale);
void scroll_set(int num, Fixed32 x, Fixed32 y);
void scroll_move(int num, Fixed32 x, Fixed32 y);
//zeroes out all tilemap vram
void scroll_clearmaps(void);
//sets bg #num's character size. either 8x8 (size = 8) or 16x16 (size = 16)
void scroll_charsize(int num, int size);
#endif
