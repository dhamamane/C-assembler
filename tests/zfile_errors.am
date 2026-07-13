; Comprehensive Error Test File for Assembler
; Each section tests different types of errors

; ========== SECTION 1: SYNTAX ERRORS ==========

; Missing comma between operands
START:  mov r1 r2

; Extra comma at the end
        add r1, r2,

; Multiple commas
        sub r1,, r2

; Missing operand after comma
        cmp r1,

; Comma but no first operand
        mov , r2

; Label without colon
LABEL1  inc r1

; Empty label
:       mov r1, r2

; ========== SECTION 2: INVALID OPERAND TYPES ==========

; Immediate as destination (invalid for most instructions)
        mov r1, #5
        add r2, #10
        sub r3, #-7
        inc #5
        clr #0

; Invalid source for lea (only direct/index allowed)
        lea #5, r1
        lea r1, r2

; Invalid addressing for jump instructions
        jmp #100
        bne r1
        jsr #50

; ========== SECTION 3: WRONG NUMBER OF OPERANDS ==========

; Too many operands
        stop r1
        rts r2, r3
        inc r1, r2
        clr r1, r2, r3

; Too few operands  
        mov r1
        add r2
        cmp
        sub

; No operands where required
        mov
        lea
        add

; Operands where none allowed
        stop r1, r2
        rts LABEL

; ========== SECTION 4: INVALID INSTRUCTIONS ==========

        mvo r1, r2
        addd r1, r2
        jmpp LABEL
        halt
        push r1
        pop r2

; ========== SECTION 5: REGISTER ERRORS ==========

; Invalid register numbers
        mov r8, r1
        add r1, r9
        sub r-1, r2
        inc r10

; Not a register format
        mov rx, r1
        add r, r2
        sub r1a, r2

; ========== SECTION 6: IMMEDIATE VALUE ERRORS ==========

; Missing number after #
        mov #, r1

; Invalid immediate format
        mov #5a, r1
        add #12.5, r2
        sub ##5, r3
        cmp #, r1

; ========== SECTION 7: LABEL ERRORS ==========

; Duplicate labels
LOOP:   mov r1, r2
LOOP:   add r1, r2

; Very long label (exceeds maximum)
VERYLONGLABELNAMETHATSHOULDEXCEEDMAXIMUM: inc r1

; Label with invalid characters
LAB-EL: mov r1, r2
LAB@EL: add r1, r2
123LAB: sub r1, r2

; Reserved word as label
mov:    inc r1
add:    dec r2

; ========== SECTION 8: STRING DIRECTIVE ERRORS ==========

; Missing quotes
.string hello world

; Unclosed string
.string "hello world

; Empty string directive
.string

; ========== SECTION 9: DATA DIRECTIVE ERRORS ==========

; Missing data
.data

; Invalid number format
.data 12.5, 10
.data 10a, 20
.data , 30

; Consecutive commas
.data 10,, 20

; Trailing comma
.data 10, 20,

; ========== SECTION 10: ENTRY/EXTERN ERRORS ==========

; Missing label
.entry
.extern

; Multiple labels on same line
.extern LAB1, LAB2

; Invalid label name
.entry 123LABEL
.extern -LABEL

; ========== SECTION 11: INDEX ADDRESSING ERRORS ==========

DATA: .mat [10][10]

; Invalid index format
        mov DATA[], r1
        mov DATA[, r2
        mov DATA[r1 r2], r3
        mov DATA[5 6], r4

; Missing brackets
        mov DATA[r1, r2
        mov DATAr1], r3

; Non-register index
        mov DATA[5][r0], r1
        mov DATA[LABEL][r0], r2


; Invalid register in index
        mov DATA[r8][r0], r1
        mov DATA[rx][r0], r2

; Space in index
        mov DATA[r1 ][r2], r3
        mov DATA[ r1][r2], r3

; ========== SECTION 12: MIXED ERRORS ==========

; Multiple errors in one line
LAB1    mov #5 #10

; Invalid instruction with wrong operands
        push #5, r1, r2

; Label and instruction errors combined
LAB-2:  mvo r8, #10

; Complex line with multiple issues
LAB3 LAB4: addd r1,, r2, r3

; ========== SECTION 13: EDGE CASES ==========

; Line with only comma
        ,

; Line with only label
ALONE:

; Multiple colons
LAB::   inc r1

; Instruction as label
stop:   stop

; Numbers as operands without #
        mov 5, r1
        add r1, 10

; Mixed valid and invalid on adjacent lines
        mov r1, r2
        mvo r2, r3
        add r1, r2


; END OF ERROR TEST FILE


