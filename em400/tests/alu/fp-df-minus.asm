.prog "alu/fp-DF-minus"

; 4
; PRE [0xa0] = 0x4000
; PRE [0xa1] = 0x0000
; PRE [0xa2] = 0x0003
; -1/10
; PRE [0xa3] = 0x9999
; PRE [0xa4] = 0x9999
; PRE [0xa5] = 0x9afd

	lf 0xa0
	df 0xa3

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0

; Z, M, V
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 1
; XPCT int(r0[2]) : 0

; -40
; XPCT hex(r1) : 0xafff
; XPCT hex(r2) : 0xffff
; XPCT hex(r3) : 0xff06

; XPCT int(sr) : 0
