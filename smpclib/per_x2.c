/****************************************
*			include files				*
****************************************/
#include	"sega_xpt.h"
#include	"sega_per.h"
#include	"per_x.h"

/****************************************
*		declare private objects			*
****************************************/
static SysPort	PortData[_MAX_PORT];

static Uint8		per_buff[SZ_BUFFER];
static PerGetPer	*per_data;
static PerMulInfo	*mul_info;

SysPort	*PER_OpenPort( void ){
	PER_LInit( PER_KD_PER, _MAX_PORT*_MAX_PERIPHERAL, SZ_PERIPHERAL, per_buff, 0 );
	
	return	PortData;
}

void	PER_GetPort( SysPort	*port ){
	SysPeripheral	*peripheral;
	
	PER_LGetPer( &per_data, &mul_info );
	
	peripheral = ( SysPeripheral * )per_data;
	
	port[0].id          = ( mul_info[0].id | mul_info[0].con );
	port[0].connectable = mul_info[0].con;
	port[0].peripheral  = peripheral;
	
	port[1].id          = ( mul_info[1].id | mul_info[1].con );
	port[1].connectable = mul_info[1].con;
	if( mul_info[0].con == 0 )
		port[1].peripheral  = &peripheral[1];
	else
		port[1].peripheral  = &peripheral[mul_info[0].con];
}

const SysDevice	*PER_GetDeviceA( const SysPort	*port, Uint32	n ){
	const SysDevice	*device = NULL;
	
	if( n < port->connectable ){
		const SysDevice	*dev = ( const SysDevice * )port->peripheral[n];
		
		if( PER_GetID( dev ) != ID_UNCONNECTED )
			device = dev;
	}
	
	return	device;
}

const SysDevice	*PER_GetDeviceR( const SysPort	*port, Uint32	n ){
	const SysDevice	*device = NULL;
	Uint32	connecters = 0;
	Uint32	i;
	
	for( i = 0; i < port->connectable; i++ ){
		const SysDevice	*dev = ( const SysDevice * )port->peripheral[i];
		
		if( PER_GetID( dev ) != ID_UNCONNECTED ){
			if( connecters == n ){
				device = dev;
				break;
			}
			
			connecters++;
		}
	}
	
	return	device;
}

