#ifndef VBLANK_H
#define VBLANK_H

#include <sega_per.h> 

extern void  SetVblank(void);

extern volatile Uint16  PadData1;
extern volatile Uint16  PadData1E;
extern volatile Uint16  PadData2;
extern volatile Uint16  PadData2E;
extern volatile Sint32  VblankFlg;

#define PAD_U	PER_DGT_U
#define PAD_D	PER_DGT_D
#define PAD_R	PER_DGT_R
#define PAD_L	PER_DGT_L
#define PAD_A	PER_DGT_A
#define PAD_B	PER_DGT_B
#define PAD_C	PER_DGT_C
#define PAD_S	PER_DGT_S
#define PAD_X	PER_DGT_X
#define PAD_Y	PER_DGT_Y
#define PAD_Z	PER_DGT_Z
#define PAD_RB	PER_DGT_TR
#define PAD_LB	PER_DGT_TL

#endif