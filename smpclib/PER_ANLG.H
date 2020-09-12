/*
**■PER_ANLG.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（アナログデバイス用）
**	
**	PER_GetAnalogXYZ()
**		標準アナログデバイスのＡ／Ｄコンバータの出力値を得る。
**	
**	SysDevAnalog
**		サターン標準アナログデバイス
*/
#if	!defined( __PER_ANLG_H )
	#define	__PER_ANLG_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	
	/****************************************
	*		declare object types			*
	****************************************/
	/*
	**■SysDevAnalog
	**	.type
	**		サターンペリフェラルタイプ（TYPE_ANALOG）
	**	.size
	**		ペリフェラルデータサイズ
	**	.data[]
	**		トリガ情報
	**	.ax
	**		Ｘ方向のＡ／Ｄコンバータの出力値（左0～右255）
	**	.ay
	**		Ｙ方向のＡ／Ｄコンバータの出力値（上0～下255）
	**	.az
	**		Ｚ方向のＡ／Ｄコンバータの出力値（下0～上255）
	**	.expended[]
	**		拡張データ
	*/
	typedef	struct	SysDevAnalog	{
		Uint8	type;
		Uint8	size;
		Uint8	data[2];
		Uint8	ax;
		Uint8	ay;
		Uint8	az;
		Uint8	expanded[1];
	} SysDevAnalog;
	
	/****************************************
	*			declare functions			*
	****************************************/
	void	PER_GetAnalogXYZ( const SysDevAnalog	*this, int	*x, int	*y, int	*z );
	
#endif

