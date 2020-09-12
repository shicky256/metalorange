/*
**■PER_KBRD.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（キーボードデバイス用）
**	
**	PER_GetKeyboardType()
**		標準キーボードデバイスのタイプを得る。
**	
**	SysDevKeyboard
**		サターン標準キーボードデバイス
*/
#if	!defined( __PER_KBRD_H )
	#define	__PER_KBRD_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*			define constants			*
	****************************************/
	/*	キーボードタイプ		*/
	#define	KB_TYPE		0x07
	#define	KB_UNKNOWN	0x07
	
	/*	SysDevKeyboard.shift	*/
	#define	K_CAPS		0x40
	#define	K_NUM		0x20
	#define	K_SCROLL	0x10
	#define	K_MAKE		0x08
	#define	K_BREAK		0x01
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■SysDevKeyboard
	**	.type
	**		サターンペリフェラルタイプ（==TYPE_KEYBOARD）
	**	.size
	**		拡張データ
	**	.data[]
	**		トリガ情報、キーボードタイプ。
	**	.shift
	**		シフトキー情報（K_CAPS,K_NUM,K_SCROLL,K_BREAK,K_MAKE）
	**	.c
	**		文字ナンバー
	**	.expended[]
	**		拡張データ
	*/
	typedef	struct	SysDevKeyboard	{
		Uint8	type;
		Uint8	size;
		Uint8	data[2];
		Uint8	shift;
		Uint8	c;
		Uint8	expanded[1];
	} SysDevKeyboard;
	
	/****************************************
	*			declare functions			*
	****************************************/
	#if	!defined( __GNUC__ )
		#define	inline
	#endif
	
	static inline int	PER_GetKeyboardType( const SysDevKeyboard	*this ){
		return	( int )( this->data[1] & KB_TYPE );
	}
	
#endif

