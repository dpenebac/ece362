.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .asciz "dpenebac"

.align 2
.global main
main:
    //bl   autotest // Uncomment this ONLY when you're not manually invoking below.
    movs r0, #1
    movs r1, #2
    movs r2, #4
    movs r3, #8
    bl   example // invoke the example subroutine
    nop

    movs r0, #35 // replace these values with examples from the prelab
    movs r1, #18
    movs r2, #23
    movs r3, #12
    bl   step31 // invoke Step 3.1
    nop

    movs r0, #10 // replace these values with examples from the prelab
    movs r1, #3
    movs r2, #18
    movs r3, #42
    bl   step32 // invoke Step 3.2
    nop

    movs r0, #24 // replace these values with examples from the prelab
    movs r1, #35
    movs r2, #52
    movs r3, #85
    bl   step33 // invoke Step 3.3
    nop

    movs r0, #29 // replace these values with examples from the prelab
    movs r1, #42
    movs r2, #93
    movs r3, #184
    bl   step41 // invoke Step 4.1
    nop

    movs r0, #0x11 // replace these values with examples from the prelab
    movs r1, #0x22
    bl   step42 // invoke Step 4.2
    nop

    movs r0, #0 // unused as an input operand
    movs r1, #16
    movs r2, #2
    movs r3, #3
    bl   step51 // invoke Step 5.1
    nop

    movs r0, #5
    bl   step52 // invoke Step 5.2
    nop


    bl   setup_portc
loop:
    bl   toggle_leds
    ldr  r0, =500000000
wait:
    subs r0,#83
    bgt  wait
    b    loop

// The main function never returns.
// It ends with the endless loop, above.

// Subroutine for example in Step 3.0
.global example
example:
    // Enter your code here
    bx   lr

// Subroutine for Step 3.1
.global step31
step31:
    // Enter your code here
    bx   lr

// Subroutine for Step 3.2
.global step32
step32:
    // Enter your code here
    bx   lr

// Subroutine for Step 3.3
.global step33
step33:
    // Enter your code here
    bx   lr

// Subroutine for Step 4.1
.global step41
step41:
    // Enter your code here
    bx   lr

// Subroutine for Step 4.2
.global step42
step42:
    // Enter your code here
    bx   lr

// Subroutine for Step 5.1
.global step51
step51:
    // Enter your code here
    bx   lr

// Subroutine for Step 5.2
.global step52
step52:
    // Enter your code here
    bx   lr

// Step 6: Type in the .equ constant initializations below

.global setup_portc
setup_portc:
    // Type in the code here.
    bx   lr

.global toggle_leds
toggle_leds:
    // Type in the code here.
    bx   lr
