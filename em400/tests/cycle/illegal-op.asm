.prog "cycle/illegal-op"

; illegal instruction resets MOD

	md 1
.data	1
	lw r1, 0
	hlt 077

.finprog

; XPCT int(sr) : 0
; XPCT int(rz[6]) : 1

; XPCT int(r1) : 0

