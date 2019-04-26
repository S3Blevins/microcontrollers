.nolist
.include "m1284def.inc"
.list

.section .text

.global ZERO, ONE

ONE:
	sbi PORTB, 1

	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1

	cbi PORTB, 1
	ret

ZERO:

	sbi PORTB, 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1

	cbi PORTB, 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	nop				; 1
	ret				; 4
