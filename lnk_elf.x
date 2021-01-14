OUTPUT_FORMAT ("elf32-sh")
OUTPUT_ARCH (sh)

SECTIONS
{
	.entry 0x06010000 :
	{
		*(.entry)
		. = ALIGN(0x20);
		_eentry = ABSOLUTE(.) ;
	}
	.text _eentry :
	{
		*(.text*)
		*(.rodata*)
		*(.strings)
		. = ALIGN(0x10) ;
		_etext = ABSOLUTE(.) ;
	}

	.tors _etext :
	{
		___ctors = . ;
		*(.ctors)
		___ctors_end = . ;
		___dtors = . ;
		*(.dtors)
		___dtors_end = . ;
		. = ALIGN(0x10) ;
		_etors = ABSOLUTE(.) ;
	}

	.data _etors :
	{
		*(.data)
		. = ALIGN(0x10) ;
		_edata = ABSOLUTE(.) ;
	}
	.bss _edata (NOLOAD) :
	{
		_bss_start = . ;
		*(.bss)
		*( COMMON )
		_bss_end = . ;
		. = ALIGN(0x10) ;
		_end = ABSOLUTE(.) ;
 	}
	.stack _end (NOLOAD) :
	{
	*(.stack)
	 _stack = . ;
	}
}