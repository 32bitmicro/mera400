.prog "ocena K-202"

; Slightly modified (see notes below) version of a test used
; for K-202 evaluation done on December 4th 1972 by
; K-202 Evaluation Committee
;
; Results: http://mera400.pl/index.php/Wydajno%C5%9B%C4%87_EM400

	lw r4, 0
	lw r3, -10		; -10 instead of 0
	lw r7, adr_1
	lw r6, adr_2
	lw r2, adr_pocz

adr_pocz:
	aw r4, [r7]
	ac r3, [r6]
	jm r2			; jm instead of uj allowing program to stop

	hlt 077

adr_1:	.data 1
adr_2:	.data 0

.finprog
