; Name: Do Le
; Student ID: 2001183621
; Course: CPE 301-1001

.ORG 0

.EQU STARTADDS = 0x300
.EQU DIV7 = 0x500
.EQU DIV3 = 0x600
.EQU DIVBOTH = 0x700
.EQU DIVNONE = 0x800

; Calculates the modulus by repeated subtraction. Overwrites @0 with the modulus.
.MACRO MOD
subtract:
	CP @0, @1
	BRLO done
	SUB @0, @1
	RJMP subtract
done:
.ENDMACRO

start:
; (PART 1). Store 200 numbers between 26 and 226 starting at 0x300.
	LDI XH, HIGH(STARTADDS)
	LDI XL, LOW(STARTADDS)
	LDI R16, 200
	LDI R17, 26
loop:
	ST X+, R17
	INC R17
	DEC R16
	BRNE loop

; (PART 2/3). Parse through the 200 numbers. If divisible by 7 store starting
; at 0x500. If divisible by 3 store starting at 0x600. If divisible by both
; store starting at 0x700. Otherwise store starting at 0x800.
; Add up all the numbers and store the sum in R18:R17:R16 register pairs.

	; Initializes the pointers for the five arrays.
	LDI XH, HIGH(STARTADDS)
	LDI XL, LOW(STARTADDS)

	; Loads register to keep track of the lower byte of memory address.
	LDI R25, LOW(DIV7)
	LDI R24, LOW(DIV3)
	LDI R23, LOW(DIVBOTH)
	LDI R22, LOW(DIVNONE)

	; Initializes result of sum by clearing all result registers.
	CLR R0
	CLR R1
	CLR R2
	CLR R3
	CLR R4
	CLR R5
	CLR R6
	CLR R7
	CLR R8
	CLR R9
	CLR R10
	CLR R11
	CLR R12
	CLR R13

	; Initializes the counter for the loop.
	LDI R20, 200		
iter:
	; Reads in the next value from the array starting at 0x300.
	LD R16, X+

	; Calculates the modulus by 7.
	MOV R17, R16
	LDI R18, 7
	MOD R17, R18

	; Calculates the modulus by 3.
	MOV R18, R16
	LDI R19, 3
	MOD R18, R19

	; If the number is divisible by 3 or 7, go to divisible to determine where the number is placed.
	; Otherwise, store in array starting at DIVNONE and jump to the endif.
	CPI R17, 0
	BREQ divisible
	CPI R18, 0
	BREQ divisible
	LDI YH, HIGH(DIVNONE)
	MOV YL, R22
	ST Y, R16
	; Adds the value to register pairs R12:R11:R10.
	ADD R10, R16
	ADC R11, R0
	ADC R12, R0

	INC R22
	RJMP endif

divisible:
	; If divisible by 7, store the number, otherwise skip to elseif0.
	CPI R17, 0			
	BRNE elseif0
	LDI YH, HIGH(DIV7)
	MOV YL, R25
	ST Y, R16
	; Adds the value to register pairs R3:R2:R1.
	ADD R1, R16
	ADC R2, R0
	ADC R3, R0

	INC R25
elseif0:
	; If divisible by 3, store the number, otherwise skip to elseif1.
	CPI R18, 0
	BRNE elseif1
	LDI YH, HIGH(DIV3)
	MOV YL, R24
	ST Y, R16
	; Adds the value to register pairs R6:R5:R4.
	ADD R4, R16
	ADC R5, R0
	ADC R6, R0

	INC R24
elseif1:
	; If divisible by both, store the number, otherwise skip to endif.
	CPI R17, 0
	BRNE endif
	CPI R18, 0
	BRNE endif
	LDI YH, HIGH(DIVBOTH)
	MOV YL, R23
	ST Y, R16
	; Adds the value to register pairs R9:R8:R7
	ADD R7, R16
	ADC R8, R0
	ADC R9, R0

	INC R23
endif:
	; Decrement the loop counter, then repeat the loop.
	DEC R20
	BRNE iter

end:
	RJMP end
