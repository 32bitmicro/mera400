
	.include awp-fp.asm

operation:
	lf r7+ARG1
	nrf
	uj r5

tests:
	.word 0, ?Z, 0
	.float 0
	.float 0
	.float 0
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077
