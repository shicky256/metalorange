/*
**■PER_POIN.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（ポインティングデバイス用）
**	
**	PER_GetPointerXY()
**		標準ポインティングデバイスの移動量を得る。
**	
**	SysDevPointer
**		サターン標準ポインティングデバイス
*/
#if	!defined( __PER_POIN_H )
	#define	__PER_POIN_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*			define constants			*
	****************************************/
	/*	SysDevPointer.status	*/
	#define	PT_OVER_Y	0x80
	#define	PT_OVER_X	0x40
	#define	PT_SIGN_Y	0x20
	#define	PT_SIGN_X	0x10
	#define	PT_START	0x08
	#define	PT_MIDDLE	0x04
	#define	PT_RIGHT	0x02
	#define	PT_LEFT		0x01
	#define	PT_TRIGGER	0x0f
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■SysDevPointer
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_POINTER）
	**	.size
	**		ペリフェラルデータサイズ
	**	.status
	**		トリガ情報、移動量の符号、移動量のオーバーフロー。
	**	.dx
	**	.dy
	**		ＸＹ方向の移動量の絶対値（0～255）
	**	.expended[]
	**		拡張データ
	*/
	typedef	struct	SysDevPointer	{
		Uint8	type;
		Uint8	size;
		Uint8	status;
		Uint8	dx;
		Uint8	dy;
		Uint8	expanded[1];
	} SysDevPointer;
	
	/****************************************
	*			declare functions			*
	****************************************/
	#if	!defined( __GNUC__ )
		#define	inline
	#endif
	
	static inline void	PER_GetPointerXY( const SysDevPointer	*this, int	*dx, int	*dy ){
		if( dx != NULL )
			*dx = ( int )( signed char )this->dx;
		if( dy != NULL )
			*dy = -( int )( signed char )this->dy;
	}
	
#endif

