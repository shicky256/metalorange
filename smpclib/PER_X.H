/*
**■PER_X.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ
**	
**	PER_OpenPort()
**		ペリフェラルライブラリを初期化する。
**	PER_GetPort()
**		ペリフェラルデータを収集する。
**		（ V-BLANK-OUT 割り込みハンドラの中でコールすること。）
**	PER_GetDeviceA()
**		収集したペリフェラルデータからデバイス情報を得る。（絶対位置指定）
**	PER_GetDeviceR()
**		収集したペリフェラルデータからデバイス情報を得る。（相対位置指定）
**	
**	SysPort
**		ペリフェラル情報
**	
**	
**	下記の３つの定数の値を適宜変更し、再コンパイルして使用してください。
**	_MAX_PORT
**		ＳＡＴＵＲＮ本体についている端子の数
**		（通常は、2 に設定しておく）
**	_MAX_PERIPHERAL
**		１つの端子に接続可能なペリフェラルの数
**		（通常は、マルチターミナルに対応するために 6 以上に設定しておく）
**	SZ_PERIPHERAL
**		１つのペリフェラルのデータサイズの最大
**	
**	SZ_BUFFER
**		ペリフェラルライブラリが使用するワークエリアのサイズです。
*/
#if	!defined( __PER_X_H )
	#define	__PER_X_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	#include	"sega_per.h"
	
	#include	"per_dev.h"
	#include	"per_dgt.h"
	#include	"per_anlg.h"
	#include	"per_poin.h"
	#include	"per_kbrd.h"
	#include	"per_md.h"
	#include	"per_trg.h"
	
	/****************************************
	*			define constants			*
	****************************************/
	#define	EXIT_SUCCESS	0
	
	#define	_MAX_PORT		2
	#define	_MAX_PERIPHERAL	6
	#define	SZ_PERIPHERAL	6
	#define	SZ_BUFFER		((_MAX_PORT*_MAX_PERIPHERAL*(SZ_PERIPHERAL+2)*2)+SZ_PERIPHERAL)
	
	#define	PID_GUN		0x0a
	
	#define	MID_GUN		0xa0
	
	#define ANALOGPAD_ID (0x16)

	#define	PER_LOCK	0x00000001
	
	/****************************************
	*		declare object types			*
	****************************************/
	typedef	Uint8	SysPeripheral[SZ_PERIPHERAL+2];
	
	typedef	struct	SysPort	{
		Uint8			id;
		Uint8			connectable;
		SysPeripheral	*peripheral;
	} SysPort;
	
	/****************************************
	*			declare functions			*
	****************************************/
	#if	!defined( __GNUC__ )
		#define	inline
	#endif
	
	static inline int	PER_GetPortID( SysPort	*port ){
		return	( int )( port->id >> 4 );
	}
	
	SysPort		*PER_OpenPort( void );
	void		PER_GetPort( SysPort	*port );
	const SysDevice	*PER_GetDeviceA( const SysPort	*port, Uint32	n );
	const SysDevice	*PER_GetDeviceR( const SysPort	*port, Uint32	n );
	
#endif

