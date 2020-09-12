/*
**■PER_MD.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（メガドライブ用ペリフェラル用）
**	
**	DevMD3B
**		メガドライブ・３ボタンパッド
**	DevMD6B
**		メガドライブ・６ボタンパッド
**	DevMDMouse
**		メガドライブ・マウス
*/
#if	!defined( __PER_MD_H )
	#define	__PER_MD_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■DevMD3B
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_MD）
	**	.size
	**		ペリフェラルデータサイズ（==1）
	**	.data[]
	**		トリガ情報
	*/
	struct	DevMD3B	{
		Uint8	type;
		Uint8	size;
		Uint8	data[1];
	};
	
	/*
	**■DevMD6B
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_MD）
	**	.size
	**		ペリフェラルデータサイズ（==2）
	**	.data[]
	**		トリガ情報
	*/
	struct	DevMD6B	{
		Uint8	type;
		Uint8	size;
		Uint8	data[2];
	};
	
	/*
	**■DevMDMouse
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_MD）
	**	.size
	**		ペリフェラルデータサイズ（==3）
	**	.status
	**		トリガ情報、移動量の符号、移動量のオーバーフロー。
	**	.dx
	**	.dy
	**		ＸＹ方向の移動量の絶対値（0～255）
	*/
	struct	DevMDMouse	{
		Uint8	type;
		Uint8	size;
		Uint8	status;
		Uint8	dx;
		Uint8	dy;
	};
	
#endif

