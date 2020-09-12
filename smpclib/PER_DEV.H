/*
**■PER_DEV.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ
**	
**	PER_GetID()
**		サターンペリフェラルＩＤを得る。
**	PER_GetType()
**		ペリフェラルタイプを得る。
**	PER_GetSize()
**		ペリフェラルデータのサイズを得る。
**	
**	SysDevice
**		サターン標準ペリフェラルの基本型
*/
#if	!defined( __PER_DEV_H )
	#define	__PER_DEV_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*			define constants			*
	****************************************/
	/*	サターンペリフェラルＩＤ	*/
	#define	ID_UNCONNECTED	0xff
	#define	ID_MD3B			0xe1
	#define	ID_MD6B			0xe2
	#define	ID_MDMOUSE		0xe3
	#define	ID_DIGITAL		0x02
	#define	ID_RACING		0x13
	#define	ID_ANALOG		0x15
	#define	ID_MOUSE		0x23
	#define	ID_KEYBOARD		0x34
	#define	ID_GUN			0xfa
	
	#define	ID_TYPE			0xf0
	#define	ID_SIZE			0x0f
	
	/*	サターンペリフェラルタイプ	*/
	#define	TYPE_DIGITAL	0x00
	#define	TYPE_ANALOG		0x10
	#define	TYPE_POINTER	0x20
	#define	TYPE_KEYBOARD	0x30
	#define	TYPE_MD			0xe0
	#define	TYPE_UNKNOWN	0xf0
	
	/*	バーチャガンのトリガ情報	*/
	#define	GUN_START		0x20
	#define	GUN_TRIGGER		0x10
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■SysDevice
	**	.type
	**		サターンペリフェラルタイプ
	**	.size
	**		ペリフェラルデータサイズ
	**	.data[]
	**		データ
	*/
	typedef	struct	SysDevice	{
		Uint8	type;
		Uint8	size;
		Uint8	data[1];
	} SysDevice;
	
	/****************************************
	*			declare functions			*
	****************************************/
	#if	!defined( __GNUC__ )
		#define	inline
	#endif
	
	static inline int	PER_GetType( const SysDevice	*this ){
		return	( int )this->type;
	}
	static inline int	PER_GetSize( const SysDevice	*this ){
		int		size = 0;
		
		if( PER_GetType( this ) != TYPE_UNKNOWN )
			size = ( int )this->size;
		
		return	size;
	}
	static inline int	PER_GetID( const SysDevice	*this ){
		return	( int )( this->type | this->size );
	}
	
#endif

