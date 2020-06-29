#ifndef GRAPHICREFS_H
#define GRAPHICREFS_H

//---background tiles---
//roarbg.c
extern Uint16 roarbg_num;
extern char roarbg_name[];
extern Uint32 roarbg_pal[];

//roarframes.c
extern Uint16 roarframes_num;
extern char roarframes_name[];
extern Uint32 roarframes_pal[];

//---sprite graphics---
//font.c
#define GRAPHIC_FONT (0)
extern Uint16 font_num;
extern Uint16 font_size;
extern Uint16 font_width;
extern Uint16 font_height;
extern char font_name[];
extern Uint32 font_pal[];
#endif
