/*
**■PER_TRG.H
**	ペリフェラルライブラリの高水準Ｉ／Ｆライブラリ（トリガ情報について）
**	
**	PER_DisguiseTrigger()
**		トリガ情報をアサインテーブルにしたがって変換する。
**	
**	PER_GetTrigger()
**		指定されたタイミングでのトリガ情報を得る。
**		（アサインテーブルが指定されていれば、それにしたがって変換する）
**	PER_GetEdge()
**		指定された２つのタイミングの間で変化したトリガ情報を得る。
**	PER_GetPressEdge()
**		指定された２つのタイミングの間で押されたトリガ情報を得る。
**	PER_GetReleaseEdge()
**		指定された２つのタイミングの間で放されたトリガ情報を得る。
**	
**	PER_SetTriggerHandler()
**		指定されたＩＤを持つペリフェラル用のトリガハンドラを登録する。
**	PER_GetTriggerHandler()
**		指定されたＩＤを持つペリフェラル用のトリガハンドラを得る。
**	
**	assigment_t
**		トリガ情報変換テーブル（アサインテーブル）
**	trigger_t
**		トリガ情報
**	trigger_handler_t
**		トリガハンドラ（デバイスからトリガ情報を取り出すためのハンドラ）
*/
#if	!defined( __PER_TRG_H )
	#define	__PER_TRG_H
	
	/****************************************
	*			include files				*
	****************************************/
	#include	"sega_xpt.h"
	#include	"sega_per.h"
	
	/****************************************
	*			define constants			*
	****************************************/
	/*
	**■trigger_t
	**	TRG_A       Ａボタン
	**	TRG_B       Ｂボタン
	**	TRG_C       Ｃボタン
	**	TRG_X       Ｘボタン
	**	TRG_Y       Ｙボタン
	**	TRG_Z       Ｚボタン
	**	TRG_L       Ｌボタン
	**	TRG_R       Ｒボタン
	**	TRG_UP      ↑
	**	TRG_DOWN    ↓
	**	TRG_LEFT    ←
	**	TRG_RIGHT   →
	**	TRG_START   スタートボタン
	**	
	**	標準ポインティングデバイスの  右  のボタンは TRG_A
	**	標準ポインティングデバイスの真ん中のボタンは TRG_C
	**	標準ポインティングデバイスの  左  のボタンは TRG_B
	**	標準ポインティングデバイスには TRG_START,A,B,C しかない。
	**	メガドライブ６Ｂパッドのモードボタンは TRG_R
	**	メガドライブ６Ｂパッドには TRG_L はない。
	**	メガドライブ３Ｂパッドには TRG_X,Y,Z,L,R はない。
	**	レーシングコントローラーには Y,Z 方向のアナログ出力はない。
	**	レーシングコントローラーには TRG_LEFT,RIGHT,L,R はない。
	**	
	**	TRG_MODE	メガドライブ６ボタンパッドのモードボタン
	**	TRG_RESET	ソフトフェアリセット
	*/
	#define	TRG_B		PER_DGT_B
	#define	TRG_C		PER_DGT_C
	#define	TRG_A		PER_DGT_A
	#define	TRG_START	PER_DGT_S
	#define	TRG_UP		PER_DGT_U
	#define	TRG_DOWN	PER_DGT_D
	#define	TRG_LEFT	PER_DGT_L
	#define	TRG_RIGHT	PER_DGT_R
	#define	TRG_L		PER_DGT_TL
	#define	TRG_Z		PER_DGT_Z
	#define	TRG_Y		PER_DGT_Y
	#define	TRG_X		PER_DGT_X
	#define	TRG_R		PER_DGT_TR
	
	#define	TRG_MODE	TRG_R
	
	#define	TRG_RESET	(TRG_START|TRG_A|TRG_B|TRG_C)
	
	#define	TRIGGER_BIT	16
	
	/****************************************
	*		declare object types			*
	****************************************/
	typedef Uint16	trigger_t;
	
	/*
	**■assignment_t
	**	メンバの順番を変更してはいけない。メンバの順番は trigger_t 用の定
	**	数 'TRG_???' のビット並びと対応している。
	*/
	typedef	struct assignment_t	{
		trigger_t	reserved1[3];
		trigger_t	l;
		trigger_t	z;
		trigger_t	y;
		trigger_t	x;
		trigger_t	r;
		trigger_t	b;
		trigger_t	c;
		trigger_t	a;
		trigger_t	start;
		trigger_t	up;
		trigger_t	down;
		trigger_t	left;
		trigger_t	right;
	} assignment_t;
	
	typedef trigger_t	( *trigger_handler_t )( const SysDevice * );
	
	/****************************************
	*			declare functions			*
	****************************************/
	trigger_t	PER_DisguiseTrigger( const assignment_t	*assignment, trigger_t	trigger );
	
	void	PER_SetTriggerHandler( int	id, trigger_handler_t	handler );
	trigger_handler_t	PER_GetTriggerHandler( int	id );
	
	trigger_t	PER_GetTrigger( const SysDevice	*this );
	trigger_t	PER_GetEdge( trigger_t	t1, trigger_t	t2 );
	trigger_t	PER_GetPressEdge( trigger_t	prev, trigger_t	current );
	trigger_t	PER_GetReleaseEdge( trigger_t	prev, trigger_t	current );
	
#endif

