/*----------------------------------------------------------------------------
 *  V_Blank.c -- V-Blank割り込み処理内ルーチンサンプル
 *  Copyright(c) 1994 SEGA
 *  Written by K.M on 1994-05-16 Ver.1.00
 *  Updated by K.M on 1994-09-21 Ver.1.00
 *
 *  UsrVblankStart()	：V-Blank開始割り込み処理サンプル
 *  UsrVblankEnd()	：V-Blank終了割り込み処理サンプル
 *
 *----------------------------------------------------------------------------
 */

#include	<machine.h>
#include	<sega_mth.h>
#include	<sega_spr.h>
#include	<sega_scl.h>
#include	<sega_xpt.h>
#include 	<sega_int.h>
#include	<sega_per.h>
#include	"pcmsys.h"
#include	"vblank.h"

typedef	struct {
	Uint8	type;
	Uint8	size;
	Uint8	data[2];
	Uint8	ax;
	Uint8	ay;
	Uint8	ar;
	Uint8	al;
} AnalogPadData;

volatile Uint16	PadData1  = 0x0000;
volatile Uint16	PadData1L = 0x0000;
volatile Uint16	PadData1E = 0x0000;
volatile Uint16	PadData2  = 0x0000;
volatile Uint16	PadData2L = 0x0000;
volatile Uint16	PadData2E = 0x0000;
Uint32	PadWorkArea[7];

volatile Sint32 perFlag;
volatile Sint32 VblankFlg;

#define ALL_BUTTONS (PAD_U | PAD_D | PAD_L | PAD_R | PAD_A | PAD_B | PAD_C \
	| PAD_X | PAD_Y | PAD_Z | PAD_S | PAD_LB | PAD_RB)

void UsrVblankIn(void);
void UsrVblankOut(void);
void CheckVblankEnd(void);

void SetVblank(void) {
	// Wait for vblank out interrupt
	
	INT_ChgMsk(INT_MSK_NULL, INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT);
	INT_SetScuFunc(INT_SCU_VBLK_OUT, CheckVblankEnd);
	INT_ChgMsk(INT_MSK_VBLK_OUT, INT_MSK_NULL);

	perFlag = 1;
	while (perFlag);
    
	PER_Init(PER_KD_PERTIM, 2, PER_ID_DGT, PER_SIZE_DGT, PadWorkArea, 0);

	// Register vblank routine
	INT_ChgMsk(INT_MSK_NULL, INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT);
	INT_SetScuFunc(INT_SCU_VBLK_IN, UsrVblankIn);
	INT_SetScuFunc(INT_SCU_VBLK_OUT, UsrVblankOut);
	INT_ChgMsk(INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT, INT_MSK_NULL);
}


void CheckVblankEnd(void) {
	perFlag = 0;
}

void UsrVblankIn(void) {
	m68k_com->start = 1;	
	SCL_VblankStart();
}

void UsrVblankOut(void) {
	SCL_VblankEnd();
	
	PerDgtInfo *pad;
	PER_GetPer((PerGetPer **)&pad);
	if (pad != NULL) {
		PadData1L = PadData1;
		PadData2L = PadData2;

		PadData1  = pad[0].data ^ 0xffff;
		PadData1E = (PadData1L ^ ALL_BUTTONS) & PadData1;
		PadData2  = pad[1].data ^ 0xffff;
		PadData2E = (PadData2L ^ ALL_BUTTONS) & PadData2;
	}
	VblankFlg = 0;
}
