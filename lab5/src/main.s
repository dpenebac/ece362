.cpu cortex-m0
.thumb
.syntax unified

// RCC configuration registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x014
.equ  GPIOCEN,  0x080000
.equ  GPIOBEN,  0x040000
.equ  GPIOAEN,  0x020000
.equ  APB1ENR,  0x01c
.equ  TIM6EN,   1<<4
.equ  TIM7EN,   1<<5
.equ  TIM14EN,  1<<8

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x0100
.equ ICER, 0x0180
.equ ISPR, 0x0200
.equ ICPR, 0x0280
.equ IPR,  0x0400
.equ TIM6_DAC_IRQn, 17
.equ TIM7_IRQn,     18
.equ TIM14_IRQn,    19

// Timer configuration registers
.equ TIM6,   0x40001000
.equ TIM7,   0x40001400
.equ TIM14,  0x40002000
.equ TIM_CR1,  0x00
.equ TIM_CR2,  0x04
.equ TIM_DIER, 0x0c
.equ TIM_SR,   0x10
.equ TIM_EGR,  0x14
.equ TIM_CNT,  0x24
.equ TIM_PSC,  0x28
.equ TIM_ARR,  0x2c

// Timer configuration register bits
.equ TIM_CR1_CEN,  1<<0
.equ TIM_DIER_UDE, 1<<8
.equ TIM_DIER_UIE, 1<<0
.equ TIM_SR_UIF,   1<<0

// GPIO configuration registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x0
.equ  PUPDR,    0xc
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//============================================================================
// enable_ports() {
// Set up the ports and pins exactly as directed.
// }
.global enable_ports
enable_ports:
	//Enables the RCC clock to GPIOB and GPIOC without affecting any other RCC clock settings for other peripherals
	//Configures pins PB0 ? PB10 to be outputs
	//Configures pins PC4 ? PC8 to be outputs
	//Configures pins PC0 ? PC3 to be inputs
	//Configures pins PC0 ? PC3 to be internally pulled low

	push {lr}

	//Enable GPIOB
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Enable GPIOC
	ldr r2, =GPIOCEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//Set PB0-PB10 to be outputs
	//output : 01
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]
	ldr r2, =0x003fffff
	bics r1, r2
	ldr r2, =0x00155555
	orrs r1, r2
	str r1, [r0, #MODER]

	//PC4-PC8 outputs
	//output : 01
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x0003ff00
	bics r1, r2
	ldr r2, =0x00015500
	orrs r1, r2
	str r1, [r0, #MODER]

	//PC0-PC3 inputs
	//input : 00
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x000000ff
	bics r1, r2
	str r1, [r0, #MODER]

	//PC0-PC3 pulled low
	//PUPDR
	//pull down : 10
	ldr r0, =GPIOC
	ldr r1, [r0, #PUPDR]
	ldr r2, =0x000000ff
	bics r1, r2
	ldr r2, =0x000000aa
	orrs r1, r2
	str r1, [r0, #PUPDR]

	pop {pc}



//============================================================================
// TIM6_ISR() {
//   TIM6->SR &= ~TIM_SR_UIF
//   if (GPIOC->ODR & (1<<8))
//     GPIOC->BRR = 1<<8;
//   else
//     GPIOC->BSRR = 1<<8;
// }
//TIM6_DAC_IRQHandler
//Explain why toggle like this instead of last lab
.global TIM6_DAC_IRQHandler
.type TIM6_DAC_IRQHandler, %function
TIM6_DAC_IRQHandler:
	push {lr}

	ldr r0, =TIM6
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF
	bics r1, r2
	str r1, [r0, #TIM_SR]

	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	ldr r2, =(1 << 8)
	ands r1, r2 //odr[8th bit] & 0x00000100 (GPIOC->odr[8th bit] == pc8)
	ldr r0, =256 //0x00000100
	cmp r1, r0
	bne else //always branches to else
if:
	ldr r0, =GPIOC
	ldr r1, [r0, #BRR] //reset
	ldr r2, =(1 << 8)
	orrs r1, r2 //so we do not modify anything else in brr
	str r1, [r0, #BRR]
	b end
else:
	ldr r0, =GPIOC
	ldr r1, [r0, #BSRR] //set
	ldr r2, =(1 << 8)
	orrs r1, r2 //so we do not modify anything else in bsrr
	str r1, [r0, #BSRR]
	b end
end:
	pop {pc}


//============================================================================
// Implement the setup_tim6 subroutine below.  Follow the instructions in the
// lab text.
.global setup_tim6 //look at RCC map to enable clk
setup_tim6:
	push {lr}
	//Enables the RCC clock for Timer 6.
	//RCC_APB1ENR bit 4
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =0x00000010
	orrs r1, r2
	str r1, [r0, #APB1ENR]


	//Configure TIM6_PSC to prescale the system clock by 48000.
	//(You should know, by now, that you should not write the value 48000 to the PSC register to do this.)
	//This will divide the 48 MHz system clock by 48000 to send a 1 kHz clock to the free-running counter of the timer.
	//HOW TO DETERMINE r2 VALUE
	ldr r0, =TIM6
	ldr r1, = 48000 - 1
	str r1, [r0, #TIM_PSC]


	//Configure the Timer 6 auto-reload register (TIM6_ARR) to have a counting period of 500.
	//With an input clock of 1 kHz, this will cause a timer update event to occur every 0.5 seconds.
	//Since we intend to raise an interrupt that will run the ISR you just wrote,
	//it will cause the PC6 LED to toggle every 0.5 seconds (or blink at a rate of 1 Hz).
	//48M / 48,000 == 1,000 / 500 == 2 Hz
	ldr r0, =TIM6
	ldr r1, =500 - 1
	str r1, [r0, #TIM_ARR]


	//Configure the Timer 6 DMA/Interrupt Enable Register (TIM6_DIER) to enable the UIE flag
	//(use the symbol TIM_DIER_UIE for this).
	//This will enable an update interrupt to occur each time the free-running c
	//ounter of the timer reaches the ARR value and starts back at zero.
	ldr r0, =TIM6
	ldr r1, [r0, #TIM_DIER]
	ldr r2, =TIM_DIER_UIE
	orrs r1, r2
	str r1, [r0, #TIM_DIER]

	//Enable the interrupt for Timer 6 in the NVIC ISER in a manner similar to how you did in lab 4.
	//MIGHT BE AN ERROR
	ldr r0, =NVIC
	ldr r1, =ISER
	ldr r2, =(1 << TIM6_DAC_IRQn)
	str r2, [r0, r1]

	//Enable Timer 6 to start counting by setting the CEN bit in the Timer 6 Control Register 1.
	//(Set TIM_CR1_CEN in TIM6_CR1.)
	//Enable Tim6 counter
	//TIM_CR1
	ldr r0, =TIM6
	ldr r1, [r0, #TIM_CR1]
	ldr r2, =0x00000001
	orrs r1, r2
	str r1, [r0, #TIM_CR1]

	pop {pc}



//============================================================================
// void show_char(int n, char c) {
//   GPIOB->ODR = ((n & 7) << 8) | font[c];
// }
.global show_char
show_char:
	//r0 = n, r1 = c
	push {lr}

	movs r2, #7
	ands r0, r2 //n = n & 7
	movs r2, #8
	lsls r0, r2 //n << 8

	ldr r2, =font
	ldrb r3, [r2, r1] //r3 = font[c]

	orrs r0, r3 //r0 = (n << 8) | font[c]

	ldr r2, =GPIOB
	str r0, [r2, #ODR]

	pop {pc}

//============================================================================
// nano_wait(int x)
// Wait the number of nanoseconds specified by x.
.global nano_wait
nano_wait:
	subs r0,#83
	bgt nano_wait
	bx lr

//============================================================================
// This function is provided for you to fill the LED matrix with AbCdEFg.
// It is a very useful function.  Study it carefully.
.global fill_alpha
fill_alpha:
	push {r4,r5,lr}
	movs r4,#0
fillloop:
	movs r5,#'A' // load the character 'A' (integer value 65)
	adds r5,r4
	movs r0,r4
	movs r1,r5
	bl   show_char
	adds r4,#1
	movs r0,#7
	ands r4,r0
	ldr  r0,=1000000
	bl   nano_wait
	b    fillloop
	pop {r4,r5,pc} // not actually reached

//============================================================================
// void drive_column(int c) {
//   c = c & 3;
//   GPIOC->BSRR = 0xf00000 | (1 << (c + 4));
// }
.global drive_column
drive_column:
	push {lr}

	movs r1, #3
	ands r0, r1 //c = c & 3

	adds r0, #4 //(c + 4)
	movs r1, #1

	lsls r1, r0 //r1 = (1 << (c + 4))

	ldr r2, =0xf00000
	orrs r1, r2

	ldr r0, =GPIOC
	str r1, [r0, #BSRR]

	pop {pc}


//============================================================================
// int read_rows(void) {
//   return GPIOC->IDR & 0xf;
// }
.global read_rows
read_rows:
	push {lr}

	ldr r0, =GPIOC
	ldr r1, [r0, #IDR]

	ldr r2, =0xf
	ands r1, r2

	movs r0, r1

	pop {pc}


//============================================================================
// char rows_to_key(int rows) {
//   int n = (col & 0x3) * 4; // or int n = (col << 30) >> 28;
//   do {
//     if (rows & 1)
//       break;
//     n ++;
//     rows = rows >> 1;
//   } while(rows != 0);
//   char c = keymap[n];
//   return c;
// }
.global rows_to_key
rows_to_key:
	push {lr}

	ldr r1, =col //r1 = @col
	ldrb r1, [r1] //r1 = col
	ldr r2, =0x3 //r2 = 0x3
	ands r1, r2 //r1 = col & 0x3
	lsls r1, #2 //r1 = (col & 0x3) * 4

	//r1 = n, r0 = rows
while:
	ldr r2, =1
	movs r3, r0
	ands r3, r2 //rows & 1
	cmp r3, #1
	bne else1 //if (rows & 1)
if1:
	b end1

else1:
	adds r1, #1 //n += 1
	lsrs r0, #1 //rows = rows >> 1

	//check to go back to while
	cmp r0, #0
	bne while

end1:
	ldr r0, =keymap
	ldrb r3, [r0, r1]
	movs r0, r3

	pop {pc}


//============================================================================
// TIM7_ISR() {
//    TIM7->SR &= ~TIM_SR_UIF
//    int rows = read_rows();
//    if (rows != 0) {
//        char key = rows_to_key(rows);
//        handle_key(key);
//    }
//    char ch = disp[col];
//    show_char(col, ch);
//    col = (col + 1) & 7;
//    drive_column(col);
// }
.global TIM7_IRQHandler
.type TIM7_IRQHandler, %function
TIM7_IRQHandler:
	push {lr}

	ldr r0, =TIM7
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF
	bics r1, r2
	str r1, [r0, #TIM_SR]

	bl read_rows //r0 = rows

	cmp r0, #0
	beq end3
if3:
	bl rows_to_key //rows_to_key(rows)
	//r0 = char key
	bl handle_key
end3:

//    char ch = disp[col];
//    show_char(col, ch);
//    col = (col + 1) & 7;
//    drive_column(col);

	ldr r0, =disp
	ldr r1, =col
	ldrb r1, [r1] //r1 = col
	ldrb r2, [r0, r1] //r2 = ch = disp[col]

	movs r0, r1 //r0 = r1 = col
	movs r1, r2 //r1 = r2 = ch = disp[col]

	sub sp, #4
	str r0, [sp]

	//r0 = col, r1 = ch, r3 = col
	bl show_char //show_char(col, ch)

	ldr r3, [sp]
	add sp, #4

	adds r3, #1 //col = col + 1
	movs r0, #7
	ands r3, r0 //col = (col + 1) & 7
	ldr r0, =col
	strb r3, [r0]
	movs r0, r3
	bl drive_column
	pop {pc}

//============================================================================
// Implement the setup_tim7 subroutine below.  Follow the instructions
// in the lab text.
.global setup_tim7
setup_tim7:
	push {lr}

	//Enable the RCC clock for TIM7.
	//You must look up how to do this so that you will be prepared for a lab practical.
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM7EN
	orrs r1, r2
	str r1, [r0, #APB1ENR]

	//Set the Prescaler and Auto-Reload Register to result in a
	//timer update event exactly once per millisecond. (i.e., a frequency of 1 kHz)
	//Determine set value
	//48 MHZ / psc / arr = rate/seconds
	//48,000,000 / 4,800 == 10,000
	//10,000 / 10 == 1,000 hz == 1 khz
	ldr r0, =TIM7
	ldr r1, =4800 - 1
	str r1, [r0, #TIM_PSC]

	//ARR
	ldr r0, =TIM7
	ldr r1, =10 - 1
	str r1, [r0, #TIM_ARR]

	//Enable the UIE bit in the DIER.
	ldr r0, =TIM7
	ldr r1, [r0, #TIM_DIER]
	ldr r2, =TIM_DIER_UIE
	orrs r1, r2
	str r1, [r0, #TIM_DIER]

	//Enable the Timer 7 interrupt in the NVIC ISER.
	ldr r0, =NVIC
	ldr r1, =ISER
	ldr r2, =(1 << TIM7_IRQn)
	str r2, [r0, r1]

	//Set the CEN bit in TIM7_CR1
	ldr r0, =TIM7
	ldr r1, [r0, TIM_CR1]
	ldr r2, =0x00000001
	orrs r1, r2
	str r1, [r0, TIM_CR1]

	pop {pc}


//============================================================================
// void handle_key(char key)
// {
//     if (key == 'A' || key == 'B' || key == 'D')
//         mode = key;
//     else if (key >= '0' && key <= '9')
//         thrust = key - '0';
// }
.global handle_key
handle_key:
	push {lr}

	//r0 = key
	//65 == 'A', 66 == 'B', 68 == 'D'
	ldr r1, =65
	cmp r1, r0
	beq if2

	ldr r1, =66
	cmp r1, r0
	beq if2

	ldr r1, =68
	cmp r1, r0
	beq if2

	b else2
if2:
	ldr r1, =mode
	strb r0, [r1]
	b end2
else2:
	//r0 = key
	cmp r0, #48 //'0'
	blt end2
	cmp r0, #57 //'9'
	bgt end2

	ldr r1, =48 //'0'
	subs r0, r1 //r0 = key - '0'
	ldr r2, =thrust
	strb r0, [r2]
	b end2
end2:
	pop {pc}


//============================================================================
// void write_display(void)
// {
//     if (mode == 'C')
//         snprintf(disp, 9, "Crashed");
//     else if (mode == 'L')
//         snprintf(disp, 9, "Landed "); // Note the extra space!
//     else if (mode == 'A')
//         snprintf(disp, 9, "ALt%5d", alt);
//     else if (mode == 'B')
//         snprintf(disp, 9, "FUEL %3d", fuel);
//     else if (mode == 'D')
//         snprintf(disp, 9, "Spd %4d", velo);
// }
.global crasheds
crasheds:
.string "Crashed"
.balign 2

.global Landeds
Landeds:
.string "Landed "
.balign 2

.global ALTs
ALTs:
.string "ALt%5d"
.balign 2

.global FUELs
FUELs:
.string "FUEL %3d"
.balign 2

.global SPDs
SPDs:
.string "Spd %4d"
.balign 2

.global write_display
write_display:
	push {lr}

	ldr r0, =mode
	ldrb r0, [r0]

	cmp r0, #'C'
	beq ifC

	cmp r0, #'L'
	beq ifL

	cmp r0, #'A'
	beq ifA

	cmp r0, #'B'
	beq ifB

	cmp r0, #'D'
	beq ifD

	b end4

ifC:
	ldr r0, =disp
	ldr r1, =9
	ldr r2, =crasheds
	bl snprintf
	b end4

ifL:
	ldr r0, =disp
	ldr r1, =9
	ldr r2, =Landeds
	bl snprintf
	b end4

ifA:
	ldr r0, =disp
	ldr r1, =9
	ldr r2, =ALTs
	ldr r3, =alt
	ldrh r3, [r3]
	bl snprintf
	b end4

ifB:
	ldr r0, =disp
	ldr r1, =9
	ldr r2, =FUELs
	ldr r3, =fuel
	ldrh r3, [r3]
	bl snprintf
	b end4

ifD:
	ldr r0, =disp
	ldr r1, =9
	ldr r2, =SPDs
	//ldr r2, [r2]
	ldr r3, =velo
	ldrh r3, [r3]
	sxth r3, r3
	bl snprintf
	b end4

end4:
	pop {pc}

//============================================================================
// void update_variables(void)
// {
//     fuel -= thrust;
//     if (fuel <= 0) {
//         thrust = 0;
//         fuel = 0;
//     }
//
//     alt += velo;
//     if (alt <= 0) { // we've reached the surface
//         if (-velo < 10)
//             mode = 'L'; // soft landing
//         else
//             mode = 'C'; // crash landing
//         return;
//     }
//
//     velo += thrust - 5;
// }
.global update_variables
update_variables:
	push {lr}

	ldr r0, =fuel //-1 == 65535 causes cmp r0, #0 to branch to end5
	ldrh r0, [r0] //r0 = fuel
	sxth r0, r0

	ldr r1, =thrust
	ldrb r1, [r1] //r1 = thrust
	sxtb r1, r1

	subs r0, r1 //r0 = fuel - thrust
	ldr r2, =fuel
	strh r0, [r2] //fuel = fuel - thrust

	cmp r0, #0
	bgt end5 //r0 == fuel > 0
if5:
	ldr r1, =thrust
	ldr r0, =fuel

	movs r2, #0
	strb r2, [r1] //thrust = 0

	movs r2, #0
	strh r2, [r0] //fuel = 0
end5:

//            alt += velo;
//            if (alt <= 0) { // we've reached the surface
//                if (-velo < 10)
//                    mode = 'L'; // soft landing
//                else
//                    mode = 'C'; // crash landing
//                return;
//            }
//
//            velo += thrust - 5;
//        }
	ldr r0, =alt
	ldrh r0, [r0] //r0 = alt
	sxth r0, r0
	ldr r1, =velo
	ldrh r1, [r1] //r1 = velo
	sxth r1, r1

	adds r0, r1 //r0 = alt + velo

	ldr r2, =alt
	strh r0, [r2] //alt = alt + velo

	cmp r0, #0
	bgt end6
	//r1 = velo
if6:
	movs r0, r1 //r0 = velo
	rsbs r0, r0, #0 //r0 = -velo
	cmp r0, #10
	bge else7
if7:
	ldr r2, =mode
	ldr r3, ='L'
	strb r3, [r2]
	b end7 //return
else7:
	ldr r2, =mode
	ldr r3, ='C'
	strb r3, [r2]
	b end7 //return
end6:
	//vel += thrust - 5
	//r1 = velo
	ldr r0, =thrust
	ldrb r0, [r0] //r0 = thrust
	sxtb r0, r0
	subs r0, #5 //r0 = thrust - 5
	adds r1, r0 //r1 = velo + thrust - 5
	ldr r2, =velo
	strh r1, [r2] //velo = velo + thrust - 5
end7:
	pop {pc}


//============================================================================
// TIM14_ISR() {
//    // acknowledge the interrupt
//    update_variables();
//    write_display();
// }
.global TIM14_IRQHandler
.type TIM14_IRQHandler, %function
TIM14_IRQHandler:
	push {lr}

	//acknowledge interrupt
	ldr r0, =TIM14
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF
	bics r1, r2
	str r1, [r0, #TIM_SR]

	bl update_variables

	bl write_display

	pop {pc}


//============================================================================
// Implement setup_tim14 as directed.
.global setup_tim14
setup_tim14:
push {lr}
	//Enables the RCC clock for Timer 6.
	//RCC_APB1ENR bit 4
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM14EN
	orrs r1, r2
	str r1, [r0, #APB1ENR]


	//Configure TIM6_PSC to prescale the system clock by 48000.
	//(You should know, by now, that you should not write the value 48000 to the PSC register to do this.)
	//This will divide the 48 MHz system clock by 48000 to send a 1 kHz clock to the free-running counter of the timer.
	//HOW TO DETERMINE r2 VALUE
	ldr r0, =TIM14
	ldr r1, = 48000 - 1
	str r1, [r0, #TIM_PSC]


	//Configure the Timer 6 auto-reload register (TIM6_ARR) to have a counting period of 500.
	//With an input clock of 1 kHz, this will cause a timer update event to occur every 0.5 seconds.
	//Since we intend to raise an interrupt that will run the ISR you just wrote,
	//it will cause the PC6 LED to toggle every 0.5 seconds (or blink at a rate of 1 Hz).
	//48M / 48,000 == 1,000 / 500 == 2 Hz
	ldr r0, =TIM14
	ldr r1, =500 - 1
	str r1, [r0, #TIM_ARR]


	//Configure the Timer 6 DMA/Interrupt Enable Register (TIM6_DIER) to enable the UIE flag
	//(use the symbol TIM_DIER_UIE for this).
	//This will enable an update interrupt to occur each time the free-running c
	//ounter of the timer reaches the ARR value and starts back at zero.
	ldr r0, =TIM14
	ldr r1, [r0, #TIM_DIER]
	ldr r2, =TIM_DIER_UIE
	orrs r1, r2
	str r1, [r0, #TIM_DIER]

	//Enable the interrupt for Timer 6 in the NVIC ISER in a manner similar to how you did in lab 4.
	ldr r0, =NVIC
	ldr r1, =ISER
	ldr r2, =(1 << TIM14_IRQn)
	str r2, [r0, r1]

	//Enable Timer 6 to start counting by setting the CEN bit in the Timer 6 Control Register 1.
	//(Set TIM_CR1_CEN in TIM6_CR1.)
	//Enable Tim6 counter
	//TIM_CR1
	ldr r0, =TIM14
	ldr r1, [r0, #TIM_CR1]
	ldr r2, =TIM_CR1_CEN
	orrs r1, r2
	str r1, [r0, #TIM_CR1]

	pop {pc}


.global login
login: .string "dpenebac" // Replace with your login.
.balign 2

.global main
main:
	//bl check_wiring
	//bl fill_alpha
	bl autotest
	bl enable_ports
	bl setup_tim6
	bl setup_tim7
	bl setup_tim14
snooze:
	wfi
	b  snooze
	// Does not return.

//============================================================================
// Map the key numbers in the history array to characters.
// We just use a string for this.
.global keymap
keymap:
.string "DCBA#9630852*741"

//============================================================================
// This table is a *font*.  It provides a mapping between ASCII character
// numbers and the LED segments to illuminate for those characters.
// For instance, the character '2' has an ASCII value 50.  Element 50
// of the font array should be the 8-bit pattern to illuminate segments
// A, B, D, E, and G.  Spread out, those patterns would correspond to:
//   .GFEDCBA
//   01011011 = 0x5b
// Accessing the element 50 of the font table will retrieve the value 0x5b.
//
.global font
font:
.space 32
.byte  0x00 // 32: space
.byte  0x86 // 33: exclamation
.byte  0x22 // 34: double quote0x77
.byte  0x76 // 35: octothorpe
.byte  0x00 // dollar
.byte  0x00 // percent
.byte  0x00 // ampersand
.byte  0x20 // 39: single quote
.byte  0x39 // 40: open paren
.byte  0x0f // 41: close paren
.byte  0x49 // 42: asterisk
.byte  0x00 // plus
.byte  0x10 // 44: comma
.byte  0x40 // 45: minus
.byte  0x80 // 46: period
.byte  0x00 // slash
.byte  0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07
.byte  0x7f, 0x67
.space 7
// Uppercase alphabet
.byte  0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x6f, 0x76, 0x30, 0x1e, 0x00, 0x38, 0x00
.byte  0x37, 0x3f, 0x73, 0x7b, 0x31, 0x6d, 0x78, 0x3e, 0x00, 0x00, 0x00, 0x6e, 0x00
.byte  0x39 // 91: open square bracket
.byte  0x00 // backslash
.byte  0x0f // 93: close square bracket
.byte  0x00 // circumflex
.byte  0x08 // 95: underscore
.byte  0x20 // 96: backquote
// Lowercase alphabet
.byte  0x5f, 0x7c, 0x58, 0x5e, 0x79, 0x71, 0x6f, 0x74, 0x10, 0x0e, 0x00, 0x30, 0x00
.byte  0x54, 0x5c, 0x73, 0x7b, 0x50, 0x6d, 0x78, 0x1c, 0x00, 0x00, 0x00, 0x6e, 0x00
.byte 0x44
.balign 2

//============================================================================
// Data structures for this experiment.
//
.data
.global col
.global disp
.global mode
.global thrust
.global fuel
.global alt
.global velo
disp: .string "Hello..."
col: .byte 0
mode: .byte 'A'
thrust: .byte 0
.balign 4
.hword 0 // put this here to make sure next hword is not word-aligned
fuel: .hword 800
.hword 0 // put this here to make sure next hword is not word-aligned
alt: .hword 4500
.hword 0 // put this here to make sure next hword is not word-aligned
velo: .hword 0
.hword 0