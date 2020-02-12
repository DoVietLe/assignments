; Name: Do Le
; Student ID: 2001183621
; Course: CPE 301-1001

.org 0x00

start:
	CLR R1			; Sets the R1 register to 0x00.
	CLR R2			; Sets the R2 register to 0xFF by first
	COM R2			; clearing it, then complementing it.

	; MULTIPLICAND
	LDI R19, 0x02	; Holds the 32-bit multiplicand.
	LDI R18, 0xAB	; Since the numbers are 32-bits, and 
	LDI R17, 0x84	; each register holds 8 bits, four
	LDI R16, 0x5C	; registers are used.
					; R19 is the most signficant byte, and R16 is the least.

	; MULTIPLIER
	LDI R23, 0x00	; Holds the 32-bit multiplier.
	LDI R22, 0x00	; R23 is the most significant byte, and
	LDI R21, 0x3B	; R20 is the least.
	LDI R20, 0x5F

	; RESULT
	LDI R31, 0x00	; Holds the result.
	LDI R30, 0x00	; Multiplying a 32-bit number by a 32-bit
	LDI R29, 0x00	; number can yield up to a 64-bit number,
	LDI R28, 0x00	; so eight registers are used.
	LDI R27, 0x00	; R31 is the most significant byte, and
	LDI R26, 0x00	; R24 is the least.
	LDI R25, 0x00
	LDI R24, 0x00

loop:
	ADD R24, R16	; Adds the lower most byte.
	ADC R25, R17	; Adds the corresponding spots to the result,
	ADC R26, R18	; respecting the carry.
	ADC R27, R19	
	ADC R28, R1		; Adds the rest of the position with zeroes,
	ADC R29, R1		; while respecting the carry.
	ADC R30, R1
	ADC R31, R1

	ADD R20, R2		; Decrements the counter by adding the 2's 
	ADC R21, R2		; complement of 1.
	ADC R22, R2
	ADC R23, R2

	CP R1, R20		; Compares the first byte of the counter to zero.
	BRNE loop		; Branches up to the loop if it's not zero.
	CP R1, R21		; Continues the process for the other three
	BRNE loop		; bytes in the counter.
	CP R1, R22
	BRNE loop
	CP R1, R23
	BRNE loop

end:
	RJMP end