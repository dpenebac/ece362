.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb

//==================================================================
// ECE 362 Lab Experiment 3
// General Purpose I/O
//==================================================================

.equ  RCC,      0x40021000
.equ  AHBENR,   0x014
.equ  GPIOAEN,  0x20000
.equ  GPIOBEN,  0x40000
.equ  GPIOCEN,  0x80000
.equ  GPIOA,    0x48000000
.equ  GPIOB,    0x48000400
.equ  GPIOC,    0x48000800
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//==========================================================
// initb:
// Enable Port B in the RCC AHBENR register and configure
// the pins as described in section 2.1 of the lab
// No parameters.
// No expected return value.
.global initb
initb:
    push    {lr}
    // Student code goes here

    //Enable Clock
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Configure Ports
	//8-11 as outputs
	//0 and 4 as inputs
	//inputs : 00
	//outputs: 01
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]

	ldr r2, =0x00ff0303 //clear all bits of ports we want to modify
	bics r1, r2
	ldr r2, =0x00550000 //set bits of ports we want to modify
	orrs r1, r2

	str r1, [r0, #MODER]

    // End of student code
    pop     {pc}

//==========================================================
// initc:
// Enable Port C in the RCC AHBENR register and configure
// the pins as described in section 2.2 of the lab
// No parameters.
// No expected return value.
.global initc
initc:
    push    {lr}
    // Student code goes here
    //Enable Clock
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOCEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Configure Ports
	//4-7 as outputs
	//0-3 as inputs
	//inputs : 00
	//outputs: 01
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	ldr r2, =0x0000ffff //clear all bits of ports we want to modify
	bics r1, r2
	ldr r2, =0x00005500 //set bits of ports we want to modify
	orrs r1, r2

	str r1, [r0, #MODER]

	//Configure PUPDR
	//0-3 as pull down
	//pull down : 10
	ldr r0, =GPIOC
	ldr r1, [r0, #PUPDR]

	ldr r2, =0x000000ff //clear all bits of ports we want to modify
	bics r1, r2
	ldr r2, =0x000000aa //set bits of ports we want to modify
	orrs r1, r2

	str r1, [r0, #PUPDR]


    // End of student code
    pop     {pc}

//==========================================================
// setn:
// Set given pin in GPIOB to given value in ODR
// Param 1 - pin number (r0)
// param 2 - value [zero or non-zero] (r1)
// No expected retern value.
.global setn
setn:
    push    {lr}
    // Student code goes here
	//turn on the bit in GPIOB_ODR, defined by r0 if r1 == 1
	//turn off the bit in GPIOB_ODR, defined by r0 if r1 == 0
	//clear bits
	cmp r1, #0
	ble else
if:
	//if r1 > 0
	//turn on the bit in GPIOB_ODR
	ldr r1, =GPIOB
	ldr r2, [r1, #BSRR] //r2 = GPIOB_BSRR address
	movs r3, #1 //r3 = 1
	lsls r3, r0 //r3 = 0x00001000 (r0 == 8)
	orrs r2, r3 //sets target bit (8) to be 1
	str r2, [r1, #BSRR]
	b endif
else:
	//if r1 <= 0
	//turn off the bit in GPIOB_ODR
	ldr r1, =GPIOB
	ldr r2, [r1, #BRR]
	movs r3, #1
	lsls r3, r0
	orrs r2, r3
	str r2, [r1, #BRR]
endif:
    // End of student code
    pop     {pc}

//==========================================================
// readpin:
// read the pin given in param 1 from GPIOB_IDR (r0)
// Param 1 - pin to read
// No expected return value.
.global readpin
readpin:
    push    {lr}
    // Student code goes here

	//store output of GPIOB_IDR[r0] in r0
	ldr r1, =GPIOB
	ldr r2, [r1, #IDR]
	movs r3, #1
	lsls r3, r0 //shift to target pin
	ands r2, r3
	movs r0, r2

    // End of student code
    pop     {pc}

//==========================================================
// buttons:
// Check the pushbuttons and turn a light on or off as
// described in section 2.6 of the lab
// No parameters.
// No return value
.global buttons
buttons:
    push    {lr}
    // Student code goes here

	movs r0, #0
	bl readpin
	movs r1, r0
	movs r0, #8
	bl setn

	movs r0, #4
	bl readpin
	movs r1, r0
	movs r0, #9
	bl setn

    // End of student code
    pop     {pc}

//==========================================================
// keypad:
// Cycle through columns and check rows of keypad to turn
// LEDs on or off as described in section 2.7 of the lab
// No parameters.
// No expected return value.
.global keypad
keypad:
    push    {lr}
    // Student code goes here

    movs r3, #8 //c = 8
for:
	ldr r0, =GPIOC
	lsls r1, r3, #4 //r1 = c << 4
	str r1, [r0, #ODR] //[gpioc_odr] = r1

	bl mysleep

	ldr r1, =GPIOC
	ldr r2, [r1, #IDR] //r2 = gpioc_idr
	movs r0, #0xf
	ands r2, r0 //r2 = r = gpioc->idr & 0xf


	push {r3}

	cmp r3, #8
	beq if2

	cmp r3, #4
	beq elseif21

	cmp r3, #2
	beq elseif22

	b else2

if2:
	movs r0, #8
	movs r1, #1
	ands r1, r2 //r1 = 1 & r
	bl setn
	b endif2
elseif21:
	movs r0, #9
	movs r1, #2
	ands r1, r2 //r1 = 2 & r
	bl setn
	b endif2
elseif22:
	movs r0, #10
	movs r1, #4
	ands r1, r2
	bl setn
	b endif2
else2:
	movs r0, #11
	movs r1, #8
	ands r1, r2
	bl setn
	b endif2
endif2:

	//check forloops
	pop {r3} //get back r3 value before setn
	lsrs r3, #1 //c >> 1
	cmp r3, #0
	bgt for //if c > 0, branch to forloop


    // End of student code
    pop     {pc}

//==========================================================
// mysleep:
// a do nothing loop so that row lines can be charged
// as described in section 2.7 of the lab
// No parameters.
// No expected return value.
.global mysleep
mysleep:
    push    {lr}
    // Student code goes here

	//dont use r3
	movs r1, #0
while:
	adds r1, #1
	cmp r1, #200
	ble while

	movs r2, #0
while2:
	adds r2, #1
	cmp r2, #200
	ble while2

	movs r0, #0
while3:
	adds r0, #1
	cmp r0, #200
	ble while3

	movs r0, #0
while4:
	adds r0, #1
	cmp r0, #200
	ble while4

	movs r0, #0
while5:
	adds r0, #1
	cmp r0, #200
	ble while5

    // End of student code
    pop     {pc}

//==========================================================
// The main subroutine calls everything else.
// It never returns.
.global main
main:
    push {lr}
    bl   autotest // Uncomment when most things are working
    //bl   initb
    //bl   initc
// uncomment one of the loops, below, when ready
//loop1:
//	bl   buttons
// 	b    loop1
loop2:
    bl   keypad
    b    loop2

    wfi
    pop {pc}
