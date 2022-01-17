.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .asciz "dpenebac"

.align 2
.global main
main:
    bl   autotest // Uncomment this ONLY when you're not manually invoking below.
    movs r0, #1
    movs r1, #2
    movs r2, #4
    movs r3, #8
    bl   example // invoke the example subroutine
    nop

    movs r0, #56 // replace these values with examples from the prelab
    movs r1, #28
    rsbs r1, r1, #0 //r1 = -(r1)
    movs r2, #33
    rsbs r2, r2, #0
    movs r3, #41
    bl   step31 // invoke Step 3.1
    nop

    movs r0, #14 // replace these values with examples from the prelab
    rsbs r0, r0, #0
    movs r1, #74
    rsbs r1, r1, #0
    movs r2, #70
    rsbs r2, r2, #0
    movs r3, #18
    rsbs r3, r3, #0
    bl   step32 // invoke Step 3.2
    nop

    movs r0, #54 // replace these values with examples from the prelab
    movs r1, #99
    movs r2, #32
    movs r3, #20
    rsbs r3, r3, #0
    bl   step33 // invoke Step 3.3
    nop

    movs r0, #11 // replace these values with examples from the prelab
    movs r1, #37
	rsbs r1, r1, #0
    movs r2, #89
    rsbs r2, r2, #0
    movs r3, #2
    bl   step41 // invoke Step 4.1
    nop

    movs r0, #83 // replace these values with examples from the prelab
    rsbs r0, r0, #0
    movs r1, #85
    movs r2, #8
    rsbs r2, r2, #0
    movs r3, #3
    rsbs r3, r3, #0
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
//R0 = R0+R1+R2+R3
.global example
example:
    // Enter your code here
    adds r0, r0, r1 //r0 = r0 + r1
    adds r0, r0, r2 //r0 = r0 + r2
    adds r0, r0, r3 //r0 = r0 + r0 + r3
    bx   lr

// Subroutine for Step 3.1
//r0 = ((r1 - r2) * (r3 + r0))
.global step31
step31:
    // Enter your code here
    subs r1, r1, r2 //r1 = r1 - r2
    adds r3, r3, r0 //r3 = r3 + r0
    muls r1, r3, r1 //r1 = r3 * r1
    movs r0, r1 //r0 = r1
    bx   lr

// Subroutine for Step 3.2
//r0 = ((r3 + r0) * (r1 - -r2))
.global step32
step32:
    // Enter your code here
	adds r3, r3, r0 //r3 = r3 + r0
	adds r1, r1, r2 //r1 = r1 + r2
	muls r3, r1, r3 //r3 = r1 * r3
	movs r0, r3 //r0 = r3
    bx   lr

// Subroutine for Step 3.3
//r0 = ((r3 * (r2 - r0)) - r1)
.global step33
step33:
    // Enter your code here
    subs r2, r2, r0 //r2 = r2 - r0
    muls r3, r2, r3 //r3 = r2 * r3
    subs r0, r3, r1 //r0 = r3 - r1
    bx   lr

// Subroutine for Step 4.1
//r0 = ((r0 & r1) ^ (r2 | r3))
.global step41
step41:
    // Enter your code here
	ands r0, r1
	orrs r2, r3
	eors r0, r2
    bx   lr

// Subroutine for Step 4.2
//r0 = ((r0 & 0xc7) - (r1 & ~0xf8))
.global step42
step42:
    // Enter your code here
    movs r2, #0xc7
    ands r0, r2
    movs r3, #0xf8
    bics  r1, r3
    subs r0, r0, r1
    bx   lr

// Subroutine for Step 5.1
//R0 = (R3 << R1) >> R2
.global step51
step51:
    // Enter your code here
    lsls r3, r3, r1
    lsrs r3, r3, r2
    movs r0, r3
    bx   lr

// Subroutine for Step 5.2
//R0 = ((R0 & ~0x1) << 3) | 5
.global step52
step52:
    // Enter your code here
    movs r1, #0x1
    bics r0, r1
    movs r1, #3
    lsls r0, r0, r1
    movs r1, #5
    orrs r0, r1
    bx   lr

// Step 6: Type in the .equ constant initializations below
.equ RCC, 			0x40021000
.equ AHBENR, 		0x14
.equ GPIOCEN,		0x00080000
.equ GPIOC,			0x48000800
.equ MODER,			0x00
.equ ODR,			0x14
.equ ENABLE6_TO_9,	0x55000
.equ PINS6_TO_9,	0x3c0

.global setup_portc
setup_portc:
    // Type in the code here.
    // Enable the RCC clock for the GPIOC peripheral
    ldr r0, =RCC
    ldr r1, [r0,#AHBENR]
    ldr r2, =GPIOCEN
    orrs r1, r2
    str r1, [r0,#AHBENR]

    // Set pins 6 to 9 to be the outputs
    ldr r0, =GPIOC
    ldr r1, [r0,#MODER]
    ldr r2, =ENABLE6_TO_9
    orrs r1, r2
    str r1, [r0,#MODER]
    // Return
    bx   lr

.global toggle_leds
toggle_leds:
    // Type in the code here.
    // Read and toggel all four pins
    ldr r0, =GPIOC
    ldr r1, [r0,#ODR]
    ldr r2, =PINS6_TO_9
    eors r1, r2
    str r1, [r0,#ODR]
    bx   lr
