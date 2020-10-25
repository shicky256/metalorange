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
#include	<SEGA_PCM.H>
#include	<sega_spr.h>
#include	<sega_scl.h>
#include	<sega_xpt.h>
#include 	<sega_int.h>
#include    "pcmsys.h"
#include	"per_x.h"

typedef	struct {
	Uint8	type;
	Uint8	size;
	Uint8	data[2];
	Uint8	ax;
	Uint8	ay;
	Uint8	ar;
	Uint8	al;
} AnalogPadData;


volatile trigger_t	PadData1  = 0x0000;
volatile trigger_t	PadData1E = 0x0000;
volatile int PadID1;
volatile Uint8 PadAnalogX1;
volatile Uint8 PadAnalogY1;
volatile Uint8 PadAnalogL1;
volatile Uint8 PadAnalogR1;
volatile trigger_t	PadData2  = 0x0000;
volatile trigger_t	PadData2E = 0x0000;

SysPort	*__port = NULL;
void UsrVblankIn(void);
void UsrVblankOut(void);

void SetVblank(void) {
	
	__port = PER_OpenPort();
	
	/* V-Blank割り込みルーチンの登録 */
	INT_ChgMsk(INT_MSK_NULL,INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT);
	INT_SetScuFunc(INT_SCU_VBLK_IN,UsrVblankIn);
	INT_SetScuFunc(INT_SCU_VBLK_OUT,UsrVblankOut);
	INT_ChgMsk(INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT,INT_MSK_NULL);
}


void UsrVblankIn(void) {
	m68k_com->start = 1;	
	SCL_VblankStart();
}

void UsrVblankOut(void) {
	SCL_VblankEnd();
	
	if( __port != NULL ){
		const SysDevice	*device;

		PER_GetPort( __port );
		
		if(( device = PER_GetDeviceR( &__port[0], 0 )) != NULL ){

			int id = PER_GetID(device);
			PadID1 = id;

			if (id == ANALOGPAD_ID) {
				const AnalogPadData *analog = (const AnalogPadData *)device;
				PadAnalogX1 = analog->ax;
				PadAnalogY1 = analog->ay;
				PadAnalogL1 = analog->al;
				PadAnalogR1 = analog->ar;
			}

			trigger_t	prev = PadData1;
			trigger_t	current = PER_GetTrigger( device );
			
			PadData1  = current;
			PadData1E = PER_GetPressEdge( prev, current );
		}
		else{
			PadData1 = PadData1E = 0;
		}
		
		if(( device = PER_GetDeviceR( &__port[1], 0 )) != NULL ){
			trigger_t	prev = PadData2;
			trigger_t	current = PER_GetTrigger( device );
			
			PadData2  = current;
			PadData2E = PER_GetPressEdge( prev, current );
		}
		else{
			PadData2 = PadData2E = 0;
		}
	}
}
