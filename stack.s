	.section .stack

	.equ	STK_SIZE, 	8192

! This is the program stack
	.align 2
	.globl _PROG_STK
_PROG_STK:
	.fill STK_SIZE


