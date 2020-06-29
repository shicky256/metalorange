#ifndef SCROLL_H
#define SCROLL_H

#define VRAM_PTR(bg) ((Uint16 *)vram[bg])
extern Uint32 vram[];

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
#endif