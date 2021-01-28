#ifndef SCROLL_H
#define SCROLL_H

#define MAP_PTR(bg) ((Uint16 *)vram[bg])
#define MAP_PTR32(bg) ((Uint32 *)vram[bg])
extern Uint32 vram[];

//number of tiles between A0 and A1
#define SCROLL_A1_OFFSET (0x1000)
//number of tiles between A0 and B1
#define SCROLL_B1_OFFSET (0x3000)

#define SCROLL_HIRES_X (704)
#define SCROLL_HIRES_Y (480)
#define SCROLL_LORES_X (352)
#define SCROLL_LORES_Y (240)

#define SCROLL_RES_LOW (0)
#define SCROLL_RES_HIGH (1)
extern int scroll_res;

#define SCROLL_HEADER16 (72)
#define SCROLL_HEADER256 (1032)

typedef struct {
    Uint8 *tile_name; //filename of .TLE file
    Uint16 tile_num; //number of tiles
    Uint32 *palette; //pointer to palette
    Uint8 *map_name; //filename of .MAP file
    Uint16 map_width; //width of map
    Uint16 map_height; //height of map
} LAYER;

void scroll_init(void);
// Loads a tile file into VRAM.
// src: where in RAM the tile file is
// dest: where in VRAM to load the tile file (or NULL for "don't load into vram")
// object: what screen to load the palette to
// palno: the color to load the palette from the file at
int scroll_loadtile(void *src, void *dest, Uint32 object, Uint16 palno);
// Returns a pointer to the start of the tile file's graphics
char *scroll_tileptr(void *buff, int *size);
// Returns a pointer to the start of the map file's graphics
char *scroll_mapptr(void *buff, int *xsize, int *ysize);
void scroll_lores(void);
void scroll_hires(void);
void scroll_scale(int num, Fixed32 scale);
void scroll_set(int num, Fixed32 x, Fixed32 y);
void scroll_move(int num, Fixed32 x, Fixed32 y);
//zeroes out all tilemap vram
void scroll_clearmaps(void);
//sets bg #num's character size
void scroll_charsize(int num, Uint8 size);
//enables/disable bg #num
void scroll_enable(int num, Uint8 state);
//sets bg #num's map size (either 1 word or 2 word)
void scroll_mapsize(int num, Uint8 size);

//enable/disable bitmap graphics (turns on nbg0, disables all other bgs)
void scroll_bitmapon();
void scroll_bitmapoff();
#endif
