.prog "alu/MW"

; PRE [910] = 0b1010010010010000
; PRE [911] = 0b0000000000000000

; PRE [912] = 0b0011010110101000
; PRE [913] = 0b0000000000000000

; PRE [914] = 0b0000000000000010
; PRE [915] = 0b0000000000000010

; PRE [916] = 0b1111111111111110
; PRE [917] = 0b1111111111111110

; PRE [918] = 0b1111111111111110
; PRE [919] = 0b0000000000000100

; PRE [920] = 0b0000000000000010
; PRE [921] = 0b1111111111111100

; PRE [922] = 0b0111111111111111
; PRE [923] = 0b0111111111111111

; PRE [924] = 0b1000000000000000
; PRE [925] = 0b1000000000000000

	lw r0, 0x2000
	lw r2, [910]
	mw 911
	rd 510
	rw r0, 610

	lw r2, [912]
	mw 913
	rd 512
	rw r0, 612

	lw r2, [914]
	mw 915
	rd 514
	rw r0, 614

	lw r2, [916]
	mw 917
	rd 516
	rw r0, 616

	lw r2, [918]
	mw 919
	rd 518
	rw r0, 618

	lw r2, [920]
	mw 921
	rd 520
	rw r0, 620

	lw r2, [922]
	mw 923
	rd 522
	rw r0, 622

	lw r2, [924]
	mw 925
	rd 524
	rw r0, 624

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin([510]) : 0b0000000000000000
; XPCT bin([511]) :                   0b0000000000000000
; XPCT bin([610]) : 0b1010000000000000

; XPCT bin([512]) : 0b0000000000000000
; XPCT bin([513]) :                   0b0000000000000000
; XPCT bin([612]) : 0b1010000000000000

; XPCT bin([514]) : 0b0000000000000000
; XPCT bin([515]) :                   0b0000000000000100
; XPCT bin([614]) : 0b0010000000000000

; XPCT bin([516]) : 0b0000000000000000
; XPCT bin([517]) :                   0b0000000000000100
; XPCT bin([616]) : 0b0010000000000000

; XPCT bin([518]) : 0b1111111111111111
; XPCT bin([519]) :                   0b1111111111111000
; XPCT bin([618]) : 0b0110000000000000

; XPCT bin([520]) : 0b1111111111111111
; XPCT bin([521]) :                   0b1111111111111000
; XPCT bin([620]) : 0b0110000000000000

; XPCT bin([522]) : 0b0011111111111111
; XPCT bin([523]) :                   0b0000000000000001
; XPCT bin([622]) : 0b0010000000000000

; XPCT bin([524]) : 0b0100000000000000
; XPCT bin([525]) :                   0b0000000000000000
; XPCT bin([624]) : 0b0010000000000000

