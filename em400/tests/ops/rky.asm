.prog "op/RKY"

; PRE KB = 17234

	rky r1

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 17234

