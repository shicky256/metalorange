/*
**■PER_DGT.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（デジタルデバイス用）
**	
**	SysDevDigital
**		サターン標準デジタルデバイス
*/
#if	!defined( __PER_DGT_H )
	#define	__PER_DGT_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■SysDevDigital
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_DIGITAL）
	**	.size
	**		ペリフェラルデータサイズ
	**	.data[]
	**		トリガ情報
	**	.expended[]
	**		拡張データ
	*/
	typedef	struct	SysDevDigital	{
		Uint8	type;
		Uint8	size;
		Uint8	data[2];
		Uint8	expanded[1];
	} SysDevDigital;
	
#endif

