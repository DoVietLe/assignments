;
; Assignment_2a.asm
;
; Created: 2/23/2020 4:30:06 PM
; Author : Owner
;

.EQU A = 236			; Calculated time for 37.5ms delay.
.EQU B = 254
.EQU A2 = 254			; Calculated time for 2s delay.
.EQU B2 = 252
.EQU C2 = 50			

.ORG 0x0000
	JMP start
.ORG 0x0002				; Sets a JMP instruction to the ISR at
	JMP flash_led		; INT0 location in the interrupt V.T.

; Replace with your application code
start:
	LDI R16, HIGH(RAMEND); Initializes the stack.
	OUT SPH, R16
	LDI R16, LOW(RAMEND)
	OUT SPL, R16

	; Sets up interrupt.
	LDI R16, 0x02
	STS EICRA, R16		; Sets INT0 to interrupt on falling edge.
	LDI R16, 0x01
	OUT EIMSK, R16		; Enabled interrupt on INT0 pin.
	SEI					; Enabled the global interrupt.

	; Sets up ports.
	LDI R17, (0x01<<2)	; Makes sure LED4 starts off.
	LDI R16, 0x00
	OUT DDRC, R16		; Sets PORTC.3 as an input port.
	OUT DDRD, R16
	LDI R16, (0x01<<3)
	OUT PORTC, R16		; Enabled the pull-up resistor on PORTC.3.
	LDI R16, (0x01<<2 | 0x01<<3)
	OUT DDRB, R16		; Sets PORTB.2 and PORTB.3 as an output pin.
	OUT PORTB, R16		; Turns LED off.


poll:
	ORI R17, (0x01<<3)	; Sets the output waveform to high.
	OUT PORTB, R17
	LDI R16, 11
	CALL delay_37500us	; Waits ~0.4125s.
	ANDI R17, ~(0x01<<3)
	OUT PORTB, R17		; Sets the output waveform to low.
	LDI R16, 9
	CALL delay_37500us	; Waits ~0.3375s.
	RJMP poll			; Keep polling.

end:
	RJMP end

; Interrupt subroutine that flashes the LED on for 2 seconds, then turns it off.
flash_led:
	ANDI R17, ~(0x01<<2)
	OUT PORTB, R17		; Sets the output of PORTB.2 to low (turns LED on).
	CALL delay_2s		; Waits ~2s.
	ORI R17, (0x01<<2)	; Sets the output of PORTB.2 to high (turns LED off).
	OUT PORTB, R17
	RETI				; Return from interrupt.

; ~2 delay 
; Delay subroutine. 
delay_2s:
	PUSH R16	
	PUSH R17		; Stores the value of R17 and R18 AND R16 on the stack,
	PUSH R18		; so values aren't overwrited.
	LDI R16, C2
delay_loop0:
	LDI R17, A2		; Reloads the value of A.
delay_loop1:
	LDI R18, B2		; Reloads the value of B.
delay_loop2:
	NOP				; Do nothing.
	NOP				
	NOP				
	NOP				
	NOP				
	NOP				
	NOP				
	DEC R18			; Decrement nested loop counter.
	BRNE delay_loop2; Loop again when counter has not reached zero.
	DEC R17			; Decrement nested loop counter.
	BRNE delay_loop1; Loop again when counter has not reached zero.
	DEC R16			; Decrements counter.
	BRNE delay_loop0; Loop again when counter has not reached zero.

	POP R18			; Pops the values of R18 and R17 AND R16 back from the stack.
	POP R17
	POP R16

	RET				; Exit the subroutine.

; ~37.5ms delay 
; Delay subroutine. The delay runs for about 600,000 cycles. The value of R16 determines 
; how many times the delay is run (kind of). 
delay_37500us:
	PUSH R17		; Saves values on the stack.
	PUSH R18
delay_again:
	LDI R17, A		; Outter loop.
loop0:
	LDI R18, B		; Nested loops.
loop1:
	NOP				; Do nothing.
	NOP		
	NOP				
	NOP				
	NOP			
	NOP			
	NOP			
	DEC R18			
	BRNE loop1		
	DEC R17		
	BRNE loop0		
	DEC R16			
	BRNE delay_again

	POP R18		; Returns values from the stack.
	POP R17

	RET			; Exits the subroutine.
