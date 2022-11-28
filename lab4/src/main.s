.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb

//==================================================================
// ECE 362 Lab Experiment 4
// Interrupts
//==================================================================

// RCC config registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x014
.equ  GPIOAEN,  0x20000
.equ  GPIOBEN,  0x40000
.equ  GPIOCEN,  0x80000
.equ  APB2ENR,  0x018
.equ  SYSCFGCOMPEN, 1

// GPIO config registers
.equ  GPIOA,    0x48000000
.equ  GPIOB,    0x48000400
.equ  GPIOC,    0x48000800
.equ  MODER,    0
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

// SYSCFG config registers
.equ SYSCFG, 0x40010000
.equ EXTICR1, 0x08
.equ EXTICR2, 0x0c
.equ EXTICR3, 0x10
.equ EXTICR4, 0x14

// External interrupt config registers
.equ EXTI,  0x40010400
.equ IMR,   0x00
.equ EMR,   0x04
.equ RTSR,  0x08
.equ FTSR,  0x0c
.equ SWIER, 0x10
.equ PR,    0x14

// Variables to register things for EXTI on pin 0
.equ EXTI_RTSR_TR0, 1<<0
.equ EXTI_IMR_MR0,  1<<0
.equ EXTI_PR_PR0,   1<<0
// Variables to register things for EXTI on pin 1
.equ EXTI_RTSR_TR1, 1<<1
.equ EXTI_IMR_MR1,  1<<1
.equ EXTI_PR_PR1,   1<<1
// Variables to register things for EXTI on pin 2
.equ EXTI_RTSR_TR2, 1<<2
.equ EXTI_IMR_MR2,  1<<2
.equ EXTI_PR_PR2,   1<<2
// Variables to register things for EXTI on pin 3
.equ EXTI_RTSR_TR3, 1<<3
.equ EXTI_IMR_MR3,  1<<3
.equ EXTI_PR_PR3,   1<<3
// Variables to register things for EXTI on pin 4
.equ EXTI_RTSR_TR4, 1<<4
.equ EXTI_IMR_MR4,  1<<4
.equ EXTI_PR_PR4,   1<<4

// SysTick counter variables....
.equ STK, 0xe000e010
.equ CSR, 0x00
.equ RVR, 0x04
.equ CVR, 0x08

// NVIC config registers
.equ NVIC, 0xe000e000
.equ ISER, 0x0100
.equ ICER, 0x0180
.equ ISPR, 0x0200
.equ ICPR, 0x0280
.equ IPR,  0x0400
.equ EXTI0_1_IRQn,5  // External interrupt number for pins 0 and 1 is IRQ 5.
.equ EXTI2_3_IRQn,6  // External interrupt number for pins 2 and 3 is IRQ 6.
.equ EXTI4_15_IRQn,7 // External interrupt number for pins 4 - 15 is IRQ 7.

// GPIO config registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//==========================================================
// nano_wait
// Loop for approximately the specified number of nanoseconds.
// Write the entire subroutine below.
.global nano_wait
nano_wait:
    subs r0,#83
    bgt  nano_wait
    bx lr


//==========================================================
// initc
// Enable the RCC clock for GPIO C and configure pins as
// described in section 2.2.1.
// Do not modify any other pin configuration.
// Parameters: none
// Write the entire subroutine below.
.global initc
initc:
	push {lr}

	//Enable Clock
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOCEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Configure Ports
	//0-3 as inputs
	//4-9 as outputs
	//inputs : 00
	//outputs : 01
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	ldr r2, =0x000fffff //clearing bits
	bics r1, r2

	ldr r2, =0x00055500 //setting 4-9 as outputs
	orrs r1, r2

	str r1, [r0, #MODER]

	//Configure pull downs
	//0 - 3 pull downs
	//pull down : 10
	ldr r0, =GPIOC
	ldr r1, [r0, #PUPDR]

	ldr r2, =0x000000ff //clear bits
	bics r1, r2

	ldr r2, =0x000000aa //set 0-3 as pull down
	orrs r1, r2

	str r1, [r0, #PUPDR]

	pop {pc}


//==========================================================
// initb
// Enable the RCC clock for GPIO B and configure pins as
// described in section 2.2.2
// Do not modify any other pin configuration.
// Parameters: none
// Write the entire subroutine below.
.global initb
initb:
	push {lr}

	//Enable Clock
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Configure Ports
	//0,2,3,4 as inputs
	//8 - 11 as outputs
	//inputs : 00
	//outputs : 01
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]

	ldr r2, =0x00ff03f3 //clearing bits
	bics r1, r2

	ldr r2, =0x00550000 //setting 8-11 as outputs
	orrs r1, r2

	str r1, [r0, #MODER]

	//Configure pull downs
	//2,3 pull downs
	//pull down : 10
	ldr r0, =GPIOB
	ldr r1, [r0, #PUPDR]

	ldr r2, =0x000000f0 //clear bits
	bics r1, r2

	ldr r2, =0x000000a0 //set 2,3 as pull down
	orrs r1, r2

	str r1, [r0, #PUPDR]

	pop {pc}


//==========================================================
// togglexn
// Change the ODR value from 0 to 1 or 1 to 0 for a specified
// pin of Port C.
// Parameters: r0 holds the base address of the GPIO port
//                to use
//             r1 holds the pin number to toggle
// Write the entire subroutine below.
.global togglexn
togglexn:
	push {lr}
	//void togglexn(GPIO_TypeDef *port, int n)
    //{
        //port->ODR = port->ODR ^ (1 << n);
    //}

	//r0 = GPIO{X}
	//r1 = pin want to change

	movs r3, #1
	lsls r3, r1

	ldr r2, [r0, #ODR]

	eors r2, r3

	str r2, [r0, #ODR]

	pop {pc}


//==========================================================
// Write the EXTI interrupt handler for pins 0 and 1 below.
// Copy the name from startup/startup_stm32.s, create a label
// of that name below, declare it to be global, and declare
// it to be a function.
// It acknowledge the pending bit for pin 0, and it should
// call togglexn(GPIOB, 8).
.global EXTI0_1_IRQHandler
.type EXTI0_1_IRQHandler, %function
EXTI0_1_IRQHandler:
	push {lr}
//It should acknowledge the interrupt by writing the value EXTI_PR_PR0 to the EXTI_PR (pending register).
//The EXTI_PR_PR0 value is a symbol defined for you in the main.s template.
//
//By EXTI_PR, we mean the PR offset from the EXTI configuration register base address.

	ldr r0, =EXTI

	ldr r1, [r0, #PR]
	ldr r2, =EXTI_PR_PR0 //acknowledge bit 0
	orrs r2, r1
	str r2, [r0, #PR]

	ldr r0, =GPIOB
	movs r1, #8
	bl togglexn

	pop {pc}


//==========================================================
// Write the EXTI interrupt handler for pins 2-3 below.
// It should acknowledge the pending bit for pin2, and it
// should call togglexn(GPIOB, 9).
.global EXTI2_3_IRQHandler
.type EXTI2_3_IRQHandler, %function
EXTI2_3_IRQHandler:
	push {lr}

	ldr r0, =EXTI

	ldr r1, [r0, #PR]
	ldr r2, =EXTI_PR_PR2 //acknowledge bit 2
	orrs r2, r1
	str r2, [r0, #PR]

	ldr r0, =GPIOB
	movs r1, #9
	bl togglexn

	pop {pc}


//==========================================================
// Write the EXTI interrupt handler for pins 4-15 below.
// It should acknowledge the pending bit for pin4, and it
// should call togglxn(GPIOB, 10).
.global EXTI4_15_IRQHandler
.type EXTI4_15_IRQHandler, %function
EXTI4_15_IRQHandler:
	push {lr}

	ldr r0, =EXTI

	ldr r1, [r0, #PR]
	ldr r2, =EXTI_PR_PR4 //acknowledge bit 4
	orrs r2, r1
	str r2, [r0, #PR]

	ldr r0, =GPIOB
	movs r1, #10
	bl togglexn

	pop {pc}


//==========================================================
// init_exti
// (1) Enable the SYSCFG subsystem, and select Port B for
//     pins 0, 2, 3, and 4.
// (2) Configure the EXTI_RTSR register so that an EXTI
//     interrupt is generated on the rising edge of
//     pins 0, 2, 3, and 4.
// (3) Configure the EXTI_IMR register so that the EXTI
//     interrupts are unmasked for pins 2, 3, and 4.
// (4) Enable the three interupts for EXTI pins 0-1, 2-3 and
//     4-15. Don't enable any other interrupts.
// Parameters: none
.global init_exti
init_exti:
	push {lr}
	// Student code goes below

	//Enable Clock
	ldr r0, =RCC
	ldr r1, [r0, #APB2ENR]
	movs r2, #1
	orrs r1, r2
	str r1, [r0, #APB2ENR]

	//Configure SYSCFG for Port B
	ldr r0, =SYSCFG
	ldr r1, [r0, #EXTICR1]
	ldr r2, =0x00001101 //setting 3, 2, 0
	orrs r2, r1
	str r2, [r0, #EXTICR1]

	ldr r1, [r0, #EXTICR2]
	ldr r2, =0x00000001 //setting 4
	orrs r2, r1
	str r2, [r0, #EXTICR2]

// (2) Configure the EXTI_RTSR register so that an EXTI
//     interrupt is generated on the rising edge of
//     pins 0, 2, 3, and 4.

	//.equ EXTI_RTSR_TR0, 1<<0

	ldr r0, =EXTI

	ldr r1, [r0, #RTSR]

	ldr r2, =EXTI_RTSR_TR0
	orrs r1, r2
	ldr r2, =EXTI_RTSR_TR2
	orrs r1, r2
	ldr r2, =EXTI_RTSR_TR3
	orrs r1, r2
	ldr r2, =EXTI_RTSR_TR4
	orrs r1, r2

	str r1, [r0, #RTSR]

// (3) Configure the EXTI_IMR register so that the EXTI
//     interrupts are unmasked for pins 2, 3, and 4.

	ldr r0, =EXTI

	ldr r1, [r0, #IMR]

	ldr r2, =EXTI_IMR_MR0
	orrs r1, r2
	ldr r2, =EXTI_IMR_MR2
	orrs r1, r2
	ldr r2, =EXTI_IMR_MR3
	orrs r1, r2
	ldr r2, =EXTI_IMR_MR4
	orrs r1, r2

	str r1, [r0, #IMR]

// (4) Enable the three interupts for EXTI pins 0-1, 2-3 and
//     4-15. Don't enable any other interrupts.

	ldr r0, =EXTI0_1_IRQn
	movs r3, #1

	lsls r3, r0

	ldr r0, =EXTI2_3_IRQn
	movs r1, #1
	lsls r1, r0

	orrs r3, r1

	ldr r0, =EXTI4_15_IRQn
	movs r1, #1
	lsls r1, r0

	orrs r3, r1

	ldr  r0, =NVIC
	ldr  r1, =ISER

	str r3, [r0, r1]

	// Student code goes above
	pop  {pc}


//==========================================================
// set_col
// Set the specified column level to logic "high.
// Set the other three three columns to logic "low".
.global set_col
set_col:
	push {r0-r3, lr}

	//void set_col(int col)
    //{
        //GPIOC->BSRR = (0xf << (4 + 16));
        //GPIOC->BSRR = (1 << (8-col));
    //}

	//r0 = col
	ldr r1, =GPIOC
	ldr r2, =0xf
	movs r3, #4
	adds r3, #16
	lsls r2, r3
	str r2, [r1, #BSRR]

	ldr r1, =GPIOC
	movs r2, #1
	movs r3, #8
	subs r3, r0
	lsls r2, r3
	str r2, [r1, #BSRR]

	pop {r0-r3, pc}


//==========================================================
// The current_col variable.
.data
.global current_col
current_col:
        .word 1
.text


//==========================================================
// SysTick_Handler
// The ISR for the SysTick interrupt.
//
.global SysTick_Handler
.type SysTick_Handler, %function
SysTick_Handler:
	push {lr}
	// Student code goes below

//       void SysTick_Handler(void)
//       {
//            int row_val = GPIOC->IDR & 0xf;
//            if (current_col == 1 && (row_val & 0x8) != 0)
//                togglexn(GPIOB, 8);
//            else if (current_col == 2 && (row_val & 0x4) != 0)
//               togglexn(GPIOB, 9);
//            else if (current_col == 3 && (row_val & 0x2) != 0)
//                togglexn(GPIOB, 10);
//            else if (current_col == 4 && (row_val & 0x1) != 0)
//                togglexn(GPIOB, 11);

//            current_col += 1;
//            if (current_col > 4)
//                current_col = 1;
//            set_col(current_col);
//        }

	ldr r0, =GPIOC
	ldr r1, [r0, #IDR]
	ldr r2, =0xf
	ands r2, r1 //r2 = row_val = gpioc->idr & 0xf
	ldr r3, =current_col //r3 = current_col
	ldr r3, [r3]
	movs r0, r2

if:
	//if (current_col == 1 && (row_val & 0x8) != 0)
	//r3 = current_col, r2 = r0 = row_val
	movs r1, #1 //r1 == 1
	cmp r3, r1 //r3 - 1
	bne elseif1 //current_col != 1 branch elseif1

	movs r1, 0x8 //r1 = 0x8
	ands r0, r1 //r0 = row_val & 0x8
	movs r1, #0 //r1 = 0
	cmp r0, r1 //r0 - 0
	beq elseif1 //if r0 == #0 branch else if 1

	ldr r0, =GPIOB //base address of GPIO port
	movs r1, #8 //
	push {r0-r3}
	bl togglexn
	pop {r0-r3}
	b endif

elseif1:
	//else if (current_col == 2 && (row_val & 0x4) != 0)
	movs r0, r2
	//r3 = current_col, r2 = r0 = row_val
	movs r1, #2
	cmp r3, r1
	bne elseif2

	movs r1, 0x4
	ands r0, r1
	movs r1, #0
	cmp r0, r1
	beq elseif2

	ldr r0, =GPIOB
	movs r1, #9
	push {r0-r3}
	bl togglexn
	pop {r0-r3}
	b endif


elseif2:
	//else if (current_col == 3 && (row_val & 0x2) != 0)
	movs r0, r2
	//r3 = current_col, r2 = r0 = row_val
	movs r1, #3
	cmp r3, r1
	bne elseif3

	movs r1, 0x2
	ands r0, r1
	movs r1, #0
	cmp r0, r1
	beq elseif3

	ldr r0, =GPIOB
	movs r1, #10
	push {r0-r3}
	bl togglexn
	pop {r0-r3}
	b endif

elseif3:
	//else if (current_col == 4 && (row_val & 0x1) != 0)
	movs r0, r2
	//r3 = current_col, r2 = r0 = row_val
	movs r1, #4
	cmp r3, r1
	bne endif

	movs r1, 0x1
	ands r0, r1
	movs r1, #0
	cmp r0, r1
	beq endif

	ldr r0, =GPIOB
	movs r1, #11
	push {r0-r3}
	bl togglexn
	pop {r0-r3}
	b endif

endif:

//            current_col += 1;
//            if (current_col > 4)
//                current_col = 1;
//            set_col(current_col);
//        }

	adds r3, #1

	movs r2, #4
	cmp r3, r2
	ble skip
	movs r3, #1
skip:
	bl set_col

	ldr r0, =current_col
	str r3, [r0]

	// Student code goes above
	pop  {pc}

//==========================================================
// init_systick
// Enable the SysTick interrupt to occur every 0.5 seconds.
// Parameters: none
.global init_systick
init_systick:
	push {lr}
	// Student code goes below
	ldr r3, =STK
	ldr r0, =375000-1
	str r0, [r3, #RVR]

	movs r0, #3
	str r0, [r3, #CSR]
	// Student code goes above
	pop  {pc}


//==========================================================
// adjust_priorities
// Set the priority for EXTI pins 2-3 interrupt to 192.
// Set the priority for EXTI pins 4-15 interrupt to 128.
// Do not adjust the priority for any other interrupts.
.global adjust_priorities
adjust_priorities:
	push {lr}
	// Student code goes below

	// Student code goes above
	pop  {pc}

//==========================================================
// The main subroutine will call everything else.
// It will never return.
.global main
main:
	bl autotest // Uncomment when most things are working
	bl initb
	bl initc
	bl init_exti
	bl init_systick
	bl adjust_priorities

endless_loop:
	ldr  r0,=GPIOC
	movs r1,#9
	bl   togglexn
	ldr  r0,=500000000
	bl   nano_wait
	b    endless_loop
