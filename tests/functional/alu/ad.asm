
	.include awp-dword.inc

operation:
	ld r7+ARG1
	ad r7+ARG2
	uj r5

tests:
	.word 0, 0, 0
	.word 0b0000000000001000, 0b1100000000000111
	.word 0b0000000000001001, 0b1100000000000011
	.word 0b0000000000010010, 0b1000000000001010

	.word 0, ?V, 0
	.word 0b0100000000000000, 0b0000000000000000
	.word 0b0100000000000000, 0b0000000000000000
	.word 0b1000000000000000, 0b0000000000000000

	.word ?V, ?MVC, 0
	.word 0b1100000000000000, 0b0000000000000000
	.word 0b1100000000000000, 0b0000000000000000
	.word 0b1000000000000000, 0b0000000000000000

	.word 0, ?ZC, 0
	.dword -1
	.dword 1
	.dword 0
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
