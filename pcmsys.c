//pcm_sys.c
//this file is compiled separately
//hopefully somewhat portable
//
#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <string.h>

#include "cd.h"
#include "pcmsys.h"


const static int logtbl[] = {
/* 0 */		0, 
/* 1 */		1, 
/* 2 */		2, 2, 
/* 4 */		3, 3, 3, 3, 
/* 8 */		4, 4, 4, 4, 4, 4, 4, 4, 
/* 16 */	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
/* 32 */	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
/* 64 */	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
/* 128 */	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};
	
#define PCM_MSK1(a)				((a)&0x0001)
#define PCM_MSK3(a)				((a)&0x0007)
#define PCM_MSK4(a)				((a)&0x000F)
#define PCM_MSK5(a)				((a)&0x001F)
#define PCM_MSK10(a)			((a)&0x03FF)

#define PCM_SCSP_FREQUENCY					(44100L)

#define PCM_CALC_OCT(smpling_rate) 											\
		((int)logtbl[PCM_SCSP_FREQUENCY / ((smpling_rate) + 1)])
		
#define PCM_CALC_SHIFT_FREQ(oct)											\
		(PCM_SCSP_FREQUENCY >> (oct))
		
#define PCM_CALC_FNS(smpling_rate, shift_freq)								\
		((((smpling_rate) - (shift_freq)) << 10) / (shift_freq))
		
#define PCM_SET_PITCH_WORD(oct, fns)										\
		((int)((PCM_MSK4(-(oct)) << 11) | PCM_MSK10(fns)))
		
		#define DRV_SYS_END (10 * 1024) //System defined safe end of driver's address space

	sysComPara * m68k_com = (sysComPara *)(SNDPRG + DRV_SYS_END);
	unsigned int * scsp_load =  (unsigned int*)(0x408 + DRV_SYS_END + 0x20); //Local loading address for sound data, is DRV_SYS_END ahead of the SNDPRG, and ahead of the communication data
	unsigned short * master_volume = (unsigned short *)(SNDRAM + 0x100400);
	short numberPCMs = 0;

//////////////////////////////////////////////////////////////////////////////

inline void smpc_wait_till_ready (void) {
   // Wait until SF register is cleared
   while(SMPC_REG_SF & 0x1) { }
}

//////////////////////////////////////////////////////////////////////////////

inline void smpc_issue_command(unsigned char cmd) {
   // Set SF register so that no other command can be issued.
   SMPC_REG_SF = 1;

   // Writing COMREG starts execution
   SMPC_REG_COMREG = cmd;
}

//////////////////////////////////////////////////////////////////////////////

void load_driver_binary(char *filename, void *buffer) {
	Sint32 file_size = cd_load_nosize(filename, buffer);
	//The immediacy of these commands is important.
	//As per SEGA technical bulletin 51, the Sound CPU is not to be turned off for more than 0.5 seconds.
	// Turn off Sound CPU
	smpc_issue_command(SMPC_CMD_SNDOFF);
	smpc_wait_till_ready();
	//
	*master_volume = 0x20F; //Set max master volume + 4mbit memory // Very important, douglath
	//TODO figure out how SBL dma works
	// memcpy((void *)SNDRAM, buffer, file_size);
	DMA_CpuMemCopy1((void *)SNDRAM, buffer, file_size);
	while (DMA_CpuResult() != DMA_CPU_END);
	// Turn on Sound CPU again
	smpc_wait_till_ready();
	smpc_issue_command(SMPC_CMD_SNDON);
	//
}
	
void load_drv(void) {
	// Make sure SCSP is set to 512k mode
	*(unsigned char *)(0x25B00400) = 0x02;

	// Clear Sound Ram
	for (int i = 0; i < 0x80000; i+=4){
		*(unsigned int *)(SNDRAM + i) = 0x00000000;
	}
	void * binary_buffer = (void*)(LWRAM + 0x80000);
	
	// Copy driver over
	load_driver_binary("SDRV.BIN", binary_buffer);

}

short calculate_bytes_per_blank(int sampleRate, int is8Bit, int isPAL) {
	int frameCount = (isPAL) ? 50 : 60;
	int sampleSize = (is8Bit) ? 8 : 16;
	return ((sampleRate * sampleSize)>>3)/frameCount;
}

short load_16bit_pcm(char *filename, int sampleRate) {
	if((int)scsp_load > 0x7F800) return -1; //Illegal PCM data address, exit

	int octr;
	int shiftr;
	int fnsr;
	Sint32 file_size = cd_load_nosize(filename, (void *)((unsigned int)scsp_load + SNDRAM));
	//make sure end of the sample is word aligned
	file_size += ((unsigned int)file_size & 1) ? 1 : 0;
	file_size += ((unsigned int)file_size & 3) ? 2 : 0;
	
	octr = PCM_CALC_OCT(sampleRate);
	shiftr = PCM_CALC_SHIFT_FREQ(octr);
	fnsr = PCM_CALC_FNS(sampleRate, shiftr);
 
	m68k_com->pcmCtrl[numberPCMs].hiAddrBits = (unsigned short)( (unsigned int)scsp_load >> 16);
	m68k_com->pcmCtrl[numberPCMs].loAddrBits = (unsigned short)( (unsigned int)scsp_load & 0xFFFF);
	
	m68k_com->pcmCtrl[numberPCMs].pitchword = PCM_SET_PITCH_WORD(octr, fnsr);
	m68k_com->pcmCtrl[numberPCMs].playsize = (file_size>>1);
	m68k_com->pcmCtrl[numberPCMs].bytes_per_blank = calculate_bytes_per_blank(sampleRate, 0, PCM_SYS_REGION); //Iniitalize as max volume
	m68k_com->pcmCtrl[numberPCMs].bitDepth = 0; //Select 16-bit
	m68k_com->pcmCtrl[numberPCMs].loopType = 0; //Initialize as non-looping
	m68k_com->pcmCtrl[numberPCMs].volume = 7; //Iniitalize as max volume


	numberPCMs++; //Increment pcm #
	scsp_load = (unsigned int *)((unsigned int )scsp_load + file_size);
	return (numberPCMs-1); //Return the PCM # this sound recieved
}

short load_8bit_pcm(char *filename, int sampleRate) {
	if( (int)scsp_load > 0x7F800) return -1; //Illegal PCM data address, exit
	
	int octr;
	int shiftr;
	int fnsr;
	
	Sint32 file_size = cd_load_nosize(filename, (void *)((unsigned int)scsp_load + SNDRAM));
	file_size += ((unsigned int)file_size & 1) ? 1 : 0;
	file_size += ((unsigned int)file_size & 3) ? 2 : 0;

	octr = PCM_CALC_OCT(sampleRate);
	shiftr = PCM_CALC_SHIFT_FREQ(octr);
	fnsr = PCM_CALC_FNS(sampleRate, shiftr);
 
	m68k_com->pcmCtrl[numberPCMs].hiAddrBits = (unsigned short)( (unsigned int)scsp_load >> 16);
	m68k_com->pcmCtrl[numberPCMs].loAddrBits = (unsigned short)( (unsigned int)scsp_load & 0xFFFF);
	
	m68k_com->pcmCtrl[numberPCMs].pitchword = PCM_SET_PITCH_WORD(octr, fnsr);
	m68k_com->pcmCtrl[numberPCMs].playsize = (file_size);
	m68k_com->pcmCtrl[numberPCMs].bytes_per_blank = calculate_bytes_per_blank(sampleRate, 1, PCM_SYS_REGION); //Iniitalize as max volume
	m68k_com->pcmCtrl[numberPCMs].bitDepth = 1; //Select 8-bit
	m68k_com->pcmCtrl[numberPCMs].loopType = 0; //Initialize as non-looping
	m68k_com->pcmCtrl[numberPCMs].volume = 7; //Iniitalize as max volume


	numberPCMs++; //Increment pcm #
	scsp_load = (unsigned int *)((unsigned int )scsp_load + file_size);
	return (numberPCMs-1); //Return the PCM # this sound recieved
}

void pcm_play(short pcmNumber, char ctrlType, char volume) {
	m68k_com->pcmCtrl[pcmNumber].sh2_permit = 1;
	m68k_com->pcmCtrl[pcmNumber].volume = volume;
	m68k_com->pcmCtrl[pcmNumber].loopType = ctrlType;
}

void pcm_parameter_change(short pcmNumber, char volume, char pan) {
	m68k_com->pcmCtrl[pcmNumber].volume = volume;
	m68k_com->pcmCtrl[pcmNumber].pan = pan;
}

void pcm_cease(short pcmNumber) {
	if(m68k_com->pcmCtrl[pcmNumber].loopType <= 0) { //If it is a volatile or protected sound, the expected control method is to mute the sound and let it end itself.
		m68k_com->pcmCtrl[pcmNumber].volume = 0;     //Protected sounds have a permission state of "until they end".
	} 
	else {
		m68k_com->pcmCtrl[pcmNumber].sh2_permit = 0; //If it is a looping sound, the control method is to command it to stop.
	}
}

