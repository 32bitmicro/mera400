.program "op/KI+FI"

; PRE [10] = 0b0110101010101010
; PRE [11] = 0b0000000000000000
; PRE [12] = 0b0000000000000000
; PRE [12] = 0b0000000000000000

	fi 10
	ki 11
	fi 12
	ki 13
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin([10]) : 0b0110101010101010
; XPCT bin([11]) : 0b0110101010101010
; XPCT bin([12]) : 0b0000000000000000
; XPCT bin([13]) : 0b0000000000000000

