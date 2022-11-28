.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.equ RCC,       0x40021000
.equ GPIOA,     0x48000000
.equ GPIOB,     0x48000400
.equ GPIOC,     0x48000800
.equ AHBENR,    0x14
.equ APB2ENR,   0x18
.equ APB1ENR,   0x1c
.equ IOPAEN,    0x20000
.equ IOPBEN,    0x40000
.equ IOPCEN,    0x80000
.equ SYSCFGCOMPEN, 1
.equ TIM3EN,    2
.equ MODER,     0
.equ OSPEEDR,   8
.equ PUPDR,     0xc
.equ IDR,       0x10
.equ ODR,       0x14
.equ BSRR,      0x18
.equ BRR,       0x28
.equ PC8,       0x100

// SYSCFG control registers
.equ SYSCFG,    0x40010000
.equ EXTICR1,   0x8
.equ EXTICR2,   0xc
.equ EXTICR3,   0x10
.equ EXTICR4,   0x14

// NVIC control registers
.equ NVIC,      0xe000e000
.equ ISER,      0x100

// External interrupt control registers
.equ EXTI,      0x40010400
.equ IMR,       0x00
.equ RTSR,      0x08
.equ PR,        0x14

.equ TIM3,      0x40000400
.equ TIMCR1,    0x00
.equ DIER,      0x0c
.equ TIMSR,     0x10
.equ PSC,       0x28
.equ ARR,       0x2c

// Popular interrupt numbers
.equ EXTI0_1_IRQn,   5
.equ EXTI2_3_IRQn,   6
.equ EXTI4_15_IRQn,  7
.equ EXTI4_15_IRQn,  7
.equ TIM2_IRQn,      15
.equ TIM3_IRQn,      16
.equ TIM6_DAC_IRQn,  17
.equ TIM7_IRQn,      18
.equ TIM14_IRQn,     19
.equ TIM15_IRQn,     20
.equ TIM16_IRQn,     21
.equ TIM17_IRQn,     22

//====================================================================
// Q1
// Just another strange recursive function.
//unsigned int recur(unsigned int x) {
//	if (x < 3)
//		return x;
//	if ((x & 0xf) == 0)
//		return 1 + recur(x - 1);
//	return recur(x >> 1) + 2;
//}
//====================================================================
.global recur
recur:
	push {lr}

	cmp r0, #3
	bhi skip
return:
	b end
skip:
	ldr r1, =0xf
	ands r1, r0 //x & 0xf
	cmp r1, #0
	bne return2

return1:
	subs r0, #1
	bl recur
	adds r0, #1
	b end

return2:
	lsrs r0, #1
	bl recur
	adds r0, #2
	b end
end:
	pop {pc}

//====================================================================
// Q2
//Write an assembly language subroutine named enable_portb that configures
//the RCC to enable the clock to GPIO Port B but leaves the other clock control bits as they were.
//====================================================================
.global enable_portb
enable_portb:
	push {lr}

	//port 18 of RCC_AHENR
	ldr r0, =0x40021000
	ldr r1, =0x14
	ldr r2, [r0, r1]
	ldr r3, =(1 << 18)
	orrs r2, r3
	str r2, [r0, r1]

	pop {pc}


//====================================================================
// Q3
//Write an assembly language subroutine named enable_portc that configures
//the RCC to enable the clock to GPIO Port C but leaves the other clock control bits as they were.
//====================================================================
.global enable_portc
enable_portc:
	push {lr}

	ldr r0, =0x40021000
	ldr r1, =0x14
	ldr r2, [r0, r1] //ahbenr
	ldr r3, =(1 << 19)
	orrs r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q4
//Pin PB3
//    Input
//    Pull-down resistor enabled
//====================================================================
.global setup_pb3
setup_pb3:
	push {lr}

	ldr r0, =0x48000400 //Modder
	ldr r1, [r0]
	ldr r2, =0x000000c0
	bics r1, r2
	str r1, [r0]

	ldr r0, =0x48000400
	ldr r1, =0x0C
	ldr r2, [r0, r1] //Pupdr
	ldr r3, =0x000000c0
	bics r2, r3
	ldr r3, =0x00000080
	orrs r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q5
// Pin Pb4
// Input
// Neither pull-down nor pull-up resistor enabled
//====================================================================
.global setup_pb4
setup_pb4:
	push {lr}

	ldr r0, =GPIOB
	ldr r1, [r0, #MODER] //Modder
	ldr r2, =0x300
	bics r1, r2
	str r1, [r0, #MODER]

	ldr r0, =0x48000400
	ldr r1, =0x0C //Pupdr
	ldr r2, [r0, r1]
	ldr r3, =0x00000300
	bics r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q6
//    PC8
//    Output
//    Output Speed: High Speed
//====================================================================
.global setup_pc8
setup_pc8:
	push {lr}

	ldr r0, =0x48000800
	ldr r1, [r0] //moder
	ldr r2, =0x00030000
	bics r1, r2
	ldr r2, =0x00010000
	orrs r1, r2
	str r1, [r0]

	ldr r0, =0x48000800
	ldr r1, =0x08 //ospeedr
	ldr r2, [r0, r1]
	ldr r3, =0x00010000
	bics r2, r3
	ldr r3, =0x00030000
	orrs r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q7
//    PC9
//    Output
//    Output Speed: Medium Speed
//====================================================================
.global setup_pc9
setup_pc9:
	push {lr}

	ldr r0, =0x48000800
	ldr r1, [r0] //moder WRONG
	ldr r2, =0x000c0000
	bics r1, r2
	ldr r2, =0x00040000
	orrs r1, r2
	str r1, [r0]

	ldr r0, =0x48000800
	ldr r1, =0x08
	ldr r2, [r0, r1]
	ldr r3, =0x000c0000
	bics r2, r3
	ldr r3, =0x00040000
	orrs r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q8
//Write an assembly language subroutine named action8 that reads the state of PB3 and PB4.
//If PB4 is low and PB3 is high, then set PC8 to 0. Otherwise, set PC8 to 1.
//To test this, you should wire a push buttons to PB3 and PB4 as you did in lab experiment 4.
//When either button is pressed, it should connect the appropriate pin to a logic high.
//The green LED should be illuminated except when PB3 is high and PB4 is low.
//====================================================================
.global action8
action8:
	push {lr}

	ldr r0, =0x48000400
	ldr r1, =0x10 //IDR
	ldr r2, [r0, r1]

	ldr r3, =0x00000008 //0000 1000 (PB4L PB3H)
	cmp r2, r3
	beq pb4Lpb3H

	beq pb4Hpb3L

pb4Hpb3L:
	//set pc8 to be 1
	ldr r0, =0x48000800
	ldr r1, =0x18 //bsrr
	ldr r2, =0x00000100
	str r2, [r0, r1]
	b end2

pb4Lpb3H:
	//set pc8 to be 0
	ldr r0, =0x48000800
	ldr r1, =0x28 //brr
	ldr r2, =0x00000100
	str r2, [r0, r1]
	b end2
end2:
	pop {pc}

//====================================================================
// Q9
// If PB3 is low and PB4 is high, then set PC9 to 1. Otherwise, set PC9 to 0.
//====================================================================
.global action9
action9:
	push {lr}

	ldr r0, =GPIOB
	ldr r1, =IDR
	ldr r2, [r0, r1] //0001 1000 18 (PB4 PB3)

	ldr r3, =0x00000010 //0001 0000
	cmp r2, r3
	beq p3lp4h

	b p3hp4l

p3lp4h:
	// set pc9 to 1
	ldr r0, =GPIOC
	ldr r1, =BSRR
	ldr r2, =0x00000200 //0010 0000 0000
	str r2, [r0, r1]
	b end3
	b end3

p3hp4l:
	// set pc9 to 0
	ldr r0, =GPIOC
	ldr r1, =BRR
	ldr r2, =0x00000200 //0010 0000 0000
	str r2, [r0, r1]
	b end3
end3:
	pop {pc}


//====================================================================
// Q10
//====================================================================
// Do everything needed to write the ISR here...

//Write an assembly language subroutine to act as the Interrupt Service Routine
//(ISR) for line 2 (pin 2 of the selected port).
//You should look up the name for this ISR in the startup/startup_stm32.s file
//and copy and paste it to avoid making any mistakes.
//It should acknowledge the interrupt by writing a 1 to the appropriate bit of
//the EXTI_PR register. It should also increment the global variable named 'counter'.
.global EXTI2_3_IRQHandler
.type EXTI2_3_IRQHandler, %function
EXTI2_3_IRQHandler:
	push {lr}

	ldr r0, =0x40010400
	ldr r1, =0x14
	ldr r2, [r0, r1]
	ldr r3, =0x00000004 //0100
	orrs r2, r3
	str r2, [r0, r1]

	ldr r0, =counter
	ldr r1, [r0] //counter
	adds r1, #1
	str r1, [r0]

	pop {pc}

//====================================================================
// Q11
//enable the system clock to the SYSCFG subsystem.

//set up the apprpriate SYSCFG external interrupt configuration register
//(see the FRM, page 177) to use pin PB2 for the interrupt source

//configure the EXTI_RTSR (see the FRM, page 224) to trigger on the rising edge of PB2

//set the EXTI_IMR to not ignore pin number 2

//configure the NVIC to enable the interrupt for the ISR
//====================================================================
.global enable_exti
enable_exti:
	push {lr}

	ldr r0, =RCC
	ldr r1, =0x18
	ldr r2, =1 //0001
	str r2, [r0, r1] ///enable clock to the syscfg subsystem

	//x001 PB[x]
	//exticr1
	//exti2 to use pb2 for interrupt
	ldr r0, =0x40010000 //sysconfig
	ldr r1, =0x08 //exticr1
	ldr r2, [r0, r1]
	ldr r3, =0x00000f00 //clear exti2
	bics r2, r3
	ldr r3, =0x00000100 //set pin 8 or exti2: 0001 PB[2]
	orrs r2, r3
	str r2, [r0, r1]

	//rising edge trigger
	ldr r0, =0x40010400 //exti
	ldr r1, =0x08 //rtsr
	ldr r2, =0x00000004 //100
	str r2, [r0, r1]

	//EXTI_IMR to not ignore pin # 2
	ldr r0, =EXTI
	ldr r1, [r0] //exti_imr
	ldr r2, =(1 << 2) //pin 2
	orrs r1, r2
	str r1, [r0]

	//configure the NVIC to enable the interrupt for the ISR
	ldr r0, =NVIC
	ldr r1, =ISER
	ldr r2, =(1 << 6) //WHY IS THIS 6
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q12
//Toggles PC9 (the blue LED).
//Acknowledges the interrupt by clearing the Timer 3 update interrupt flag.
//====================================================================
// Do everything needed to write the ISR here...
.global TIM3_IRQHandler
.type TIM3_IRQHandler, %function
TIM3_IRQHandler:
	push {lr}

	ldr r0, =TIM3
	ldr r1, [r0, #TIMSR]
	ldr r2, =1
	bics r1, r2
	str r1, [r0, #TIMSR]

	ldr r0, =GPIOC
	ldr r1, =ODR
	ldr r2, [r0, r1]
	ldr r3, =(1 << 9) //pc9
	eors r2, r3
	str r2, [r0, r1]

	pop {pc}

//====================================================================
// Q13
//Enables the system clock to the timer 3 subsystem.
//Configures the Auto-Reload Register and Prescaler of Timer 3 so that an update event occurs exactly four times per second.
//Set the DIER of Timer 3 so that an interrupt occurs on an update event.
//Write the appropriate bit to the NVIC ISER so that the interrupt for the Timer 3 update event is enabled.
//Enable the counter for Timer 3.
//====================================================================
.global enable_tim3
enable_tim3:
	push {lr}

	//Enables the RCC clock for Timer 3.
	//RCC_APB1ENR bit 4
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM3EN
	orrs r1, r2
	str r1, [r0, #APB1ENR]

	ldr r0, =TIM3
	ldr r1, =48000 - 1
	str r1, [r0, #PSC]

	ldr r1, =250 - 1
	str r1, [r0, #ARR] //4 times per second

	ldr r0, =TIM3
	ldr r1, [r0, #DIER]
	ldr r2, =1
	orrs r1, r2
	str r1, [r0, #DIER]

	//Enable the interrupt for Timer 3
	ldr r0, =NVIC
	ldr r1, =ISER
	ldr r2, =(1 << 16)
	str r2, [r0, r1]

	//Enable Tim3 counter
	//TIM_CR1
	ldr r0, =TIM3
	ldr r1, [r0, #TIMCR1]
	ldr r2, =1
	orrs r1, r2
	str r1, [r0, #TIMCR1]

	pop {pc}
