/****************************************
*			include files				*
****************************************/
#include	"per_x.h"

/****************************************
*		declare private objects			*
****************************************/
static trigger_t	GetTriggerDigital( const SysDevice	*device );
static trigger_t	GetTriggerAnalog( const SysDevice	*device );
static trigger_t	GetTriggerPointer( const SysDevice	*device );
static trigger_t	GetTriggerKeyboard( const SysDevice	*device );
static trigger_t	GetTriggerMD3B( const SysDevice	*device );
static trigger_t	GetTriggerMD6B( const SysDevice	*device );
static trigger_t	GetTriggerMDMouse( const SysDevice	*device );
static trigger_t	GetTriggerUnknown( const SysDevice	*device );

static trigger_handler_t	GetTrigger[] = {
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerDigital, GetTriggerDigital,
	GetTriggerDigital, GetTriggerDigital, GetTriggerDigital, GetTriggerDigital,
	GetTriggerDigital, GetTriggerDigital, GetTriggerDigital, GetTriggerDigital,
	GetTriggerDigital, GetTriggerDigital, GetTriggerDigital, GetTriggerDigital,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerAnalog, GetTriggerAnalog,
	GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog,
	GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog,
	GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog, GetTriggerAnalog,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerPointer, GetTriggerPointer,
	GetTriggerPointer, GetTriggerPointer, GetTriggerPointer, GetTriggerPointer,
	GetTriggerPointer, GetTriggerPointer, GetTriggerPointer, GetTriggerPointer,
	GetTriggerPointer, GetTriggerPointer, GetTriggerPointer, GetTriggerPointer,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerKeyboard, GetTriggerKeyboard,
	GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard,
	GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard,
	GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard, GetTriggerKeyboard,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerMD3B, GetTriggerMD6B, GetTriggerMDMouse,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
	GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown, GetTriggerUnknown,
};

/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		PER_SetTriggerHandler( id, handler );							*
*																		*
*		指定のＩＤを持つペリフェラル用のトリガハンドラを設定します。	*
*																		*
*				・・・・・												*
*		ここでは「指定のＩＤを持つデバイス用のハンドラを設定している」	*
*		ということに注目してください。「指定のタイプ」ではありません	*
*		ので、注意してください。										*
*																		*
*		ここで設定した関数（ハンドラ）は、PER_GetTrigger() に指定の		*
*		ＩＤを持つデバイス情報が渡されたときに、コールされます。		*
*																		*
*																		*
*	【parameters】														*
*																		*
*		int					id;											*
*		trigger_handler_t	handler;									*
*																		*
*		<id> は、サターンペリフェラルＩＤ								*
*		<handler> は、トリガハンドラ									*
*																		*
*																		*
*	【return value】													*
*																		*
*		なし															*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		GetTrigger	influence	トリガハンドラのテーブル				*
*																		*
*																		*
************************************************************************/
void	PER_SetTriggerHandler( int	id, trigger_handler_t	handler ){
	GetTrigger[id] = handler;
}

/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		handler = PER_GetTriggerHandler( id );							*
*																		*
*		指定のＩＤを持つペリフェラル用のトリガハンドラを得ます。		*
*																		*
*																		*
*	【parameters】														*
*																		*
*		int		id;														*
*																		*
*		サターンペリフェラルＩＤ										*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_handler_t	handler;									*
*																		*
*		トリガハンドラ													*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		GetTrigger	reference	トリガハンドラのテーブル				*
*																		*
*																		*
************************************************************************/
trigger_handler_t	PER_GetTriggerHandler( int	id ){
	return	GetTrigger[id];
}

/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerDigital( &device );							*
*																		*
*		デジタルデバイスからトリガ情報を得ます。						*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerDigital( const SysDevice	*device ){
	const SysDevDigital	*this = ( const SysDevDigital * )device;
	
	trigger_t	data_l = ( trigger_t )( ~this->data[1] ) & 0x00ff;
	trigger_t	data_h = ( trigger_t )( ~this->data[0] ) & 0x00ff;
	
	return	( trigger_t )(( data_h << 8 ) | data_l );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerAnalog( &device );							*
*																		*
*		アナログデバイスからトリガ情報を得ます。						*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerAnalog( const SysDevice	*device ){
	const SysDevAnalog	*this = ( const SysDevAnalog * )device;
	
	trigger_t	data_l = ( trigger_t )( ~this->data[1] ) & 0x00ff;
	trigger_t	data_h = ( trigger_t )( ~this->data[0] ) & 0x00ff;
	
	return	( trigger_t )(( data_h << 8 ) | data_l );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerPointer( &device );							*
*																		*
*		ポインティングデバイスからトリガ情報を得ます。					*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerPointer( const SysDevice	*device ){
	const SysDevPointer	*this = ( const SysDevPointer * )device;
	
	trigger_t	data_h = ( trigger_t )( this->status & PT_TRIGGER );
	
	return	( trigger_t )( data_h << 8 );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerKeyboard( &device );						*
*																		*
*		キーボードデバイスからトリガ情報を得ます。						*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerKeyboard( const SysDevice	*device ){
	const SysDevKeyboard	*this = ( const SysDevKeyboard * )device;
	
	trigger_t	data_l = ( trigger_t )( ~this->data[1] ) & 0x00ff;
	trigger_t	data_h = ( trigger_t )( ~this->data[0] ) & 0x00ff;
	
	return	( trigger_t )(( data_h << 8 ) | data_l );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerMD3B( &device );							*
*																		*
*		メガドライブ用３ボタンパッドからトリガ情報を得ます。			*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerMD3B( const SysDevice	*device ){
	const struct DevMD3B	*this = ( const struct DevMD3B * )device;
	
	trigger_t	data_h = ( trigger_t )( ~this->data[0] ) & 0x00ff;
	
	return	( trigger_t )( data_h << 8 );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerMD6B( &device );							*
*																		*
*		メガドライブ用６ボタンパッドからトリガ情報を得ます。			*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerMD6B( const SysDevice	*device ){
	const struct DevMD6B	*this = ( const struct DevMD6B * )device;
	
	trigger_t	data_l = ( trigger_t )( ~this->data[1] ) & 0x00ff;
	trigger_t	data_h = ( trigger_t )( ~this->data[0] ) & 0x00ff;
	
	return	( trigger_t )(( data_h << 8 ) | data_l );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerMDMouse( &device );							*
*																		*
*		メガドライブ用マウス（なぜか、サターン用のシャトルマウスも）	*
*		からトリガ情報を得ます。										*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerMDMouse( const SysDevice	*device ){
	const struct DevMDMouse	*this = ( const struct DevMDMouse * )device;
	
	trigger_t	data_h = ( trigger_t )( this->status & PT_TRIGGER );
	
	return	( trigger_t )( data_h << 8 );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		trigger = GetTriggerUnknown( &device );							*
*																		*
*		不明なデバイスからトリガ情報を得ます。							*
*																		*
*		常に 0 を返します。												*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		常に 0 を返します。												*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
static trigger_t	GetTriggerUnknown( const SysDevice	*device ){
	return	0;
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		trigger = PER_GetTrigger( device );								*
*																		*
*		デバイスの種類に応じてトリガ情報を得ます。						*
*																		*
*		PER_SetTriggerHandler() によって登録されているＩＤを持つデバ	*
*		イスからトリガ情報を取り出すには、同じく PER_SetTriggerHandler()*
*		によって登録されている関数（ハンドラ）を用います。そういった	*
*		ものがない場合には、デフォルトで用意されている関数（ハンドラ）	*
*		を用います。この場合、予め対応している入力ペリフェラルは下記	*
*		のものだけですので、それ以外の入力ペリフェラルからトリガ情報	*
*		を得たい場合には、PER_SetTriggerHandler() を使用してください。	*
*																		*
*		デフォルトで対応している入力ペリフェラル						*
*		デジタルデバイス												*
*			サターンペリフェラルＩＤとして 0x02 - 0x0f を返すもの		*
*		アナログデバイス												*
*			サターンペリフェラルＩＤとして 0x12 - 0x1f を返すもの		*
*		ポインティングデバイス											*
*			サターンペリフェラルＩＤとして 0x22 - 0x2f を返すもの		*
*		キーボードデバイス												*
*			サターンペリフェラルＩＤとして 0x33 - 0x3f を返すもの		*
*		メガドライブ用３Ｂパッド										*
*			サターンペリフェラルＩＤとして 0xe1 を返すもの				*
*		メガドライブ用６Ｂパッド										*
*			サターンペリフェラルＩＤとして 0xe2 を返すもの				*
*		メガドライブ用マウス											*
*			サターンペリフェラルＩＤとして 0xe3 を返すもの				*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevice	*device;										*
*																		*
*		デバイス情報													*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	trigger;											*
*																		*
*		トリガ情報を返します。											*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
trigger_t	PER_GetTrigger( const SysDevice	*this ){
	trigger_t	trigger = 0;
	
	if( this != NULL ){
		int					id = PER_GetID( this );
		trigger_handler_t	handler = PER_GetTriggerHandler( id );
		
		if( handler != NULL )
			trigger = handler( this );
	}
	
	return	trigger;
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		edge = PER_GetEdge( t1, t2 );									*
*																		*
*		トリガのエッジ情報を得ます。									*
*																		*
*		エッジ情報とは <t1> と <t2> の２つのタイミングのあいだで変化	*
*		したトリガの情報のことをいいます。								*
*																		*
*																		*
*	【parameters】														*
*																		*
*		trigger_t	t1;													*
*		trigger_t	t2;													*
*																		*
*		<t1> と <t2> はトリガ情報。										*
*		<t1> と <t2> の時間的な順序は問いません。						*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	edge;												*
*																		*
*		トリガのエッジ情報を返します。									*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
trigger_t	PER_GetEdge( trigger_t	t1, trigger_t	t2 ){
	return	( trigger_t )( t1 ^ t2 );
}
/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		edge = PER_GetPressEdge( prev, current );						*
*																		*
*		トリガのプレスエッジ（押下エッジ）情報を得ます。				*
*																		*
*		プレスエッジ情報とは <prev> と <current> の２つのタイミング		*
*		のあいだにリリース状態からプレス状態に変化したトリガの情報の	*
*		ことをいいます。												*
*																		*
*																		*
*	【parameters】														*
*																		*
*		trigger_t	prev;												*
*		trigger_t	current;											*
*																		*
*		<prev> と <current> は共にトリガ情報。							*
*		時間的に（ <prev> ＜ <current> ）という関係を期待しています。	*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	edge;												*
*																		*
*		トリガのプレスエッジ情報を返します。							*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
trigger_t	PER_GetPressEdge( trigger_t	prev, trigger_t	current ){
	trigger_t	edge = PER_GetEdge( prev, current );
	
	return	( trigger_t )( current & edge );
}

/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		edge = PER_GetReleaseEdge( prev, current );						*
*																		*
*		トリガのリリースエッジ（解放エッジ）情報を得ます。				*
*																		*
*		リリースエッジ情報とは <prev> と <current> の２つのタイミン		*
*		グのあいだにプレス状態からリリース状態に変化したトリガの情報	*
*		のことをいいます。												*
*																		*
*																		*
*	【parameters】														*
*																		*
*		trigger_t	prev;												*
*		trigger_t	current;											*
*																		*
*		<prev> と <current> は共にトリガ情報。							*
*		時間的に（ <prev> ＜ <current> ）という関係を期待しています。	*
*																		*
*																		*
*	【return value】													*
*																		*
*		trigger_t	edge;												*
*																		*
*		トリガのリリースエッジ情報を返します。							*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
trigger_t	PER_GetReleaseEdge( trigger_t	prev, trigger_t	current ){
	trigger_t	edge = PER_GetEdge( prev, current );
	
	return	( trigger_t )( ~current & edge );
}

#define	ANALOG_ILLEGAL	0xff
#define	ANALOG_ORIGIN	0x7f

/*@*/
/************************************************************************
*																		*
*	【synopsis】														*
*																		*
*		#include	"per_x.h"											*
*																		*
*		PER_GetAnalogXYZ( analog, &x, &y, &z );							*
*																		*
*		アナログデバイスからＸＹＺ方向のＡ／Ｄコンバータの出力値を得	*
*		ます。															*
*																		*
*																		*
*	【parameters】														*
*																		*
*		const SysDevAnalog	*analog;									*
*		int		*x;														*
*		int		*y;														*
*		int		*z;														*
*																		*
*		<analog> はアナログデバイス情報。								*
*		<x> <y> <z> はそれぞれＸＹＺ方向のＡ／Ｄコンバータの出力値。	*
*																		*
*		<x> <y> <z> に NULL を渡すとそこには何も返されません。			*
*																		*
*		<x> には（右）-127 ～ +127（左）が返されます。					*
*		<y> には（上）-127 ～ +127（下）が返されます。					*
*		<z> には（下）-127 ～ +127（上）が返されます。					*
*																		*
*																		*
*	【return value】													*
*																		*
*		なし															*
*																		*
*																		*
*	【influence/reference objects】										*
*																		*
*		なし															*
*																		*
*																		*
************************************************************************/
void	PER_GetAnalogXYZ( const SysDevAnalog	*this, int	*x, int	*y, int	*z ){
	int		ax = 0;
	int		ay = 0;
	int		az = 0;
	
	if( this->ax != ANALOG_ILLEGAL )
		ax = ( int )this->ax - ANALOG_ORIGIN;
	if( this->ay != ANALOG_ILLEGAL )
		ay = ( int )this->ay - ANALOG_ORIGIN;
	if( this->az != ANALOG_ILLEGAL )
		az = ( int )this->az - ANALOG_ORIGIN;
	
	if( x != NULL )
		*x = ax;
	if( y != NULL )
		*y = ay;
	if( z != NULL )
		*z = az;
}

