
	lw r1, 0b1011101011101010
	lw r7, 0b1110111010111010
	bs r1, 0b1010101010101010
	hlt 040
	bs r1, 0b0000001010101000
	hlt 077
	hlt 040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
