.program "cpu/pre-mod-reset"

; legal instruction resets MOD

	md 1
	md 1
	md 1

	hlt 074

.endprog

; XPCT int(sr) : 0

; XPCT int(mod) : 0

