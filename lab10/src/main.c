/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"

/*
Enable the RCC clocks to GPIOC and GPIOD.

Do all the steps necessary to configure pin PC12 to be routed to USART5_TX.

Do all the steps necessary to configure pin PD2 to be routed to USART5_RX.

Enable the RCC clock to the USART5 peripheral.

Configure USART5 as follows:
    (First, disable it by turning off its UE bit.)

    Set a word size of 8 bits.

    Set it for one stop bit.

    Set it for no parity.

    Use 16x oversampling.

    Use a baud rate of 115200 (115.2 kbaud). Refer to table 96 of the Family Reference Manual,
    or simply divide the system clock rate by 115200.

    Enable the transmitter and the receiver by setting the TE and RE bits.

    Enable the USART.

    Finally, you should wait for the TE and RE bits to be acknowledged by checking that TEACK
    and REACK bits are both set in the ISR. This indicates that the USART is ready to transmit and receive.

 */
void init_usart5(void) {

    /*
    Enable the RCC clocks to GPIOC and GPIOD.

    Do all the steps necessary to configure pin PC12 to be routed to USART5_TX.

    Do all the steps necessary to configure pin PD2 to be routed to USART5_RX.

    Enable the RCC clock to the USART5 peripheral.
    */

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;

    /*
    RX : PC12, AF2 : DIO 0
    TX : PD2,  AF2 : DIO 1
     */

    //10 alternate function
    GPIOC->MODER |= 2 << (12 * 2);
    GPIOD->MODER |= 2 << (2 * 2);

    GPIOC->AFR[1] |= 2 << (4 * 4);
    GPIOD->AFR[0] |= 2 << (2 * 4);

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    /*
    Configure USART5 as follows:
    (First, disable it by turning off its UE bit.)

    Set a word size of 8 bits.

    Set it for one stop bit.

    Set it for no parity.

    Use 16x oversampling.

    Use a baud rate of 115200 (115.2 kbaud). Refer to table 96 of the Family Reference Manual,
    or simply divide the system clock rate by 115200.

    Enable the transmitter and the receiver by setting the TE and RE bits.

    Enable the USART.

    Finally, you should wait for the TE and RE bits to be acknowledged by checking that TEACK
    and REACK bits are both set in the ISR. This indicates that the USART is ready to transmit and receive.

    */

    USART5->CR1 &= ~(USART_CR1_UE);

    USART5->CR1 &= ~((1 << 28) | (1 << 12));

    USART5->CR2 &= ~(3 << 12);

    USART5->CR1 &= ~(USART_CR1_PCE);

    USART5->CR1 &= ~(USART_CR1_OVER8);

    //Brr ==  0x1A1
    USART5->BRR |= 0x1A1;

    USART5->CR1 |= USART_CR1_TE | USART_CR1_RE;

    USART5->CR1 |= USART_CR1_UE;

    while ( USART_ISR_TEACK != (USART5->ISR & USART_ISR_TEACK) &&
            USART_ISR_REACK != (USART5->ISR & USART_ISR_REACK)
    );

    return;
}

//#define STEP21
#if defined(STEP21)
int main(void)
{
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

//#define STEP22
#if defined(STEP22)
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
/*
In _io_putchar(), if the character passed as the argument c is a '\n' first write a '\r' to the USART5->TDR.
*/

int __io_putchar(int c) {

    if (c == '\n') {
        while(!(USART5->ISR & USART_ISR_TXE)) {}
        USART5->TDR = '\r';
    }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

/*
In _io_getchar(), if the character read, c, is a carriage return ('\r'), change it to a linefeed ('\n').
Also in _io_getchar(), echo the character read to the output by calling _io_putchar(c) just before it is returned.
*/

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE)) { }
    char c = USART5->RDR;

    if (c == '\r') {
        c = '\n';
    }

    __io_putchar(c);

    return c;
}

int main() {
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

//#define STEP23 23
#if defined (STEP23)
#include <fifo.h>
#include <tty.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
/*
In _io_putchar(), if the character passed as the argument c is a '\n' first write a '\r' to the USART5->TDR.
*/

int __io_putchar(int c) {

    if (c == '\n') {
        while(!(USART5->ISR & USART_ISR_TXE)) {}
        USART5->TDR = '\r';
    }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

/*
In _io_getchar(), if the character read, c, is a carriage return ('\r'), change it to a linefeed ('\n').
Also in _io_getchar(), echo the character read to the output by calling _io_putchar(c) just before it is returned.
*/

int __io_getchar(void) {
    char c;
    c = line_buffer_getchar();
    return (c);
}

int main() {
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

//#define STEP24 24
#if defined (STEP24)
#include <fifo.h>
#include <tty.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

/*
Raise an interrupt every time the receive data register becomes not empty.
Remember to set the proper bit in the NVIC ISER as well.
Note that the name of the bit to set is difficult to determine. It is USART3_8_IRQn.

Trigger a DMA operation every time the receive data register becomes not empty.
 */

/*
CMAR should be set to the address of serfifo.

CPAR should be set to the address of the USART5 RDR.

CNDTR should be set to FIFOSIZE.

The DIRection of copying should be from peripheral to memory.

Neither the total-completion nor the half-transfer interrupt should be enabled.

Both the MSIZE and the PSIZE should be set for 8 bits.

MINC should be set to increment the CMAR.

PINC should not be set so that CPAR always points at the USART5 RDR.

Enable CIRCular transfers.

Do not enable MEM2MEM transfers.

Set the Priority Level to highest.

Finally, make sure that the channel is enabled for operation.
 */
void enable_tty_interrupt(void) {

    //receive data register becomes not empty RXNE, RXNEIE
    USART5->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0] |= 1 << USART3_8_IRQn;

    USART5->CR3 |= USART_CR3_DMAR; //transmitter or reciever?

    //DMA stuff
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~(DMA_CCR_EN);

    DMA2_Channel2->CMAR = (uint32_t) serfifo;

    DMA2_Channel2->CPAR = (uint32_t) &USART5->RDR; //USART5 RDR, offset 0x24, + 0x4000 5000

    DMA2_Channel2->CNDTR = FIFOSIZE;

    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;

    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE);

    DMA2_Channel2->CCR |= DMA_CCR_MINC;

    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;

    DMA2_Channel2->CCR |= DMA_CCR_CIRC;

    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;

    DMA2_Channel2->CCR |= DMA_CCR_EN;

    return;
}

//2.4.3
/*
All it needs to do is check if the input_fifo contains a newline.
While it does not, it should do an inline assembly WFI:
    asm volatile ("wfi"); // wait for an interrupt

If it does contain a newline, it should remove the first character from the fifo and return it.
Use the line_buffer_getchar() subroutine in tty.c as a template for creating this new subroutine.

Update __io_getchar() to call interrupt_getchar() instead of line_buffer_getchar().
*/
int interrupt_getchar(void) {
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi");
    }

    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

void USART3_4_5_6_7_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

/*
In _io_putchar(), if the character passed as the argument c is a '\n' first write a '\r' to the USART5->TDR.
*/

int __io_putchar(int c) {

    if (c == '\n') {
        while(!(USART5->ISR & USART_ISR_TXE)) {}
        USART5->TDR = '\r';
    }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

/*
In _io_getchar(), if the character read, c, is a carriage return ('\r'), change it to a linefeed ('\n').
Also in _io_getchar(), echo the character read to the output by calling _io_putchar(c) just before it is returned.
*/

int __io_getchar(void) {
    char c;
    c = interrupt_getchar();
    return (c);
}

int main() {
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif


//#define STEP5 5
#if defined (STEP5)
#include <fifo.h>
#include <tty.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {

    //receive data register becomes not empty RXNE, RXNEIE
    USART5->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0] |= 1 << USART3_8_IRQn;

    USART5->CR3 |= USART_CR3_DMAR; //transmitter or reciever?

    //DMA stuff
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~(DMA_CCR_EN);

    DMA2_Channel2->CMAR = (uint32_t) serfifo;

    DMA2_Channel2->CPAR = (uint32_t) &USART5->RDR; //USART5 RDR, offset 0x24, + 0x4000 5000

    DMA2_Channel2->CNDTR = FIFOSIZE;

    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;

    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE);

    DMA2_Channel2->CCR |= DMA_CCR_MINC;

    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;

    DMA2_Channel2->CCR |= DMA_CCR_CIRC;

    DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;

    DMA2_Channel2->CCR |= DMA_CCR_EN;

    return;
}

int interrupt_getchar(void) {
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi");
    }

    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

void USART3_4_5_6_7_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

int __io_putchar(int c) {

    if (c == '\n') {
        while(!(USART5->ISR & USART_ISR_TXE)) {}
        USART5->TDR = '\r';
    }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    char c;
    c = interrupt_getchar();
    return (c);
}

#include "commands.h"
int main() {
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    command_shell();
}

//step5

void init_sdcard_spi(void) {

    return;
}


void enable_sdcard(void) {

    return;
}

void disable_sdcard(void) {

    return;
}

void sdcard_high_speed(void) {

    return;
}









#endif