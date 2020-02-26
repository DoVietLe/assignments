;
; Assignment_2a.asm
;
; Created: 2/23/2020 4:30:06 PM
; Author : Owner
;

.EQU A = 236
.EQU B = 254

start:
	LDI R16, HIGH(RAMEND); Initialize the stack pointer.
	OUT SPH, R16
	LDI R16, LOW(RAMEND)
	OUT SPL, R16
	
	LDI R16, (0x01<<3)	; Sets PORTB.3 as an output.
	OUT DDRB, R16

wave:
	LDI R17, (0x01<<3)	; Sets the output waveform to high.
	OUT PORTB, R17
	LDI R16, 11
	CALL delay			; Waits ~0.4125s.
	LDI R17, 0x00
	OUT PORTB, R17		; Sets the output waveform to low.
	LDI R16, 9
	CALL delay			; Waits ~0.3375s.
	RJMP wave

end:
	RJMP end


; ~37.5ms delay 
; Delay subroutine. The delay runs for about 600,000 cycles. The value of R16 determines 
; how many times the delay is run (kind of). 
delay:
	PUSH R17		; Pushes values onto the stack to save.
	PUSH R18
delay_again:
	LDI R17, A		; Runs delay again.
loop0:
	LDI R18, B		; Reinitialize the inner loop.
loop1:
	NOP				; Do nothing.
	NOP				
	NOP				
	NOP				
	NOP				
	NOP				
	NOP				
	DEC R18			; Decrease inner most loop counter.
	BRNE loop1		; Branch if counter isn't up.
	DEC R17			; Decreases middle loop counter.
	BRNE loop0		; Branch if counter isn't up.
	DEC R16			; Decreases loop counter.
	BRNE delay_again; Branch if counter isn't up.

	POP R18			; Returns values from the stack.
	POP R17

	RET				; Exit the subroutine.

; Cycles = 1 + 10abc + 3ac + 2c
