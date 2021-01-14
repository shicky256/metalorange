!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! File: entry.s
! Creator: Benjamin Lee
! Date: 5/13/94 (Friday the 13th)
!
! This is the entry point for the code.  It is real simple,
! just set the SP ... do any other esoteric stuff we need,
! then launch main()
!
!
! Copyright (c) 1994 by Sega of America, Inc.
!
! Disclaimer:
! This code is sample code. Sega of America, Inc.
! and the author(s) accept no responsibility
! for any problems which may arise from using
! this code.
!
! This file may be used and/or modified
! provided the disclaimer mentioned above is
! maintained, and the appropriate Non Disclosure
! Agreement with Sega of America, Inc. and the
! party or parties using/modifying the code exist.
! This file may not be distributed to any party/parties
! which do not have the appropriate Non Disclosure
! Agreement with Sega of America, Inc.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	.section .entry

	.align 2
	.globl _entry
_entry:

	mov.l	stk_LOC, r15	! load stack
	mov	#0xf0,r1

! Disable interrupts, temporarily

	extu.b	r1, r1
	ldc	r1, sr

	xor	r8, r8
	xor	r9, r9
	xor	r10, r10
	xor	r11, r11
	xor	r12, r12
	xor	r13, r13
	xor	r14, r14
	ldc	r14, gbr			! Clear out gbr
	lds	r14, mach			! Clear out mach
	lds	r14, macl			! Clear out macl
	mov.l	bss_E, r2
	mov.l	bss_S, r1
	add	#-1, r2
	cmp/hi	r1, r2				! bss_E > bss_S
	bf.s	doMain
	nop
done:
	cmp/hi	r1, r2				! bss_E > bss_S
	mov.b	r14, @r1
	bt.s	done
	add	#1, r1
doMain:

! Reenable interrupts

	mov	#0,r0
	ldc	r0,sr

	mov.l	main_LOC, r0
	sts.l	pr, @-r15
	jsr	@r0
	nop
loop:
	bra 	loop
	nop

	.align 2
stk_LOC:
	.long _stack
main_LOC:
	.long _main
bss_S:
	.long _bss_start
bss_E:
	.long _bss_end

	.end			

