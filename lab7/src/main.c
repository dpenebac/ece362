#include "stm32f0xx.h"
#include <math.h>   // for M_PI

void nano_wait(int);

// 16-bits per digit.
// The most significant 8 bits are the digit number.
// The least significant 8 bits are the segments to illuminate.
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];
// Print an 8-character string on the 8 digits
void print(const char str[]);
// Print a floating-point value.
void printfloat(float f);


//============================================================================
// enable_ports()
/*
Enables the RCC clock to GPIOB and GPIOC without affecting any other RCC clock settings for other peripherals
Configures pins PB0 ? PB10 to be outputs
Configures pins PC4 ? PC7 to be outputs
Configures pins PC4 ? PC7 to have output type open-drain (using the OTYPER)
Configures pins PC0 ? PC3 to be inputs
Configures pins PC0 ? PC3 to be internally pulled high
*/
//============================================================================
void enable_ports(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    //PB0 - PB10 to be outputs
    GPIOB->MODER &= ~(
            GPIO_MODER_MODER0 |
            GPIO_MODER_MODER1 |
            GPIO_MODER_MODER2 |
            GPIO_MODER_MODER3 |
            GPIO_MODER_MODER4 |
            GPIO_MODER_MODER5 |
            GPIO_MODER_MODER6 |
            GPIO_MODER_MODER7 |
            GPIO_MODER_MODER8 |
            GPIO_MODER_MODER9 |
            GPIO_MODER_MODER10
    );

    GPIOB->MODER |= (
            GPIO_MODER_MODER0_0 |
            GPIO_MODER_MODER1_0 |
            GPIO_MODER_MODER2_0 |
            GPIO_MODER_MODER3_0 |
            GPIO_MODER_MODER4_0 |
            GPIO_MODER_MODER5_0 |
            GPIO_MODER_MODER6_0 |
            GPIO_MODER_MODER7_0 |
            GPIO_MODER_MODER8_0 |
            GPIO_MODER_MODER9_0 |
            GPIO_MODER_MODER10_0
    );

    //PC4 - PC7 to be outputs
    GPIOC->MODER &= ~(0x0000ff00);
    GPIOC->MODER |= (0x00005500);

    //PC4 - PC7 to have output type open-drain
    GPIOC->OTYPER |= (
            GPIO_OTYPER_OT_4 |
            GPIO_OTYPER_OT_5 |
            GPIO_OTYPER_OT_6 |
            GPIO_OTYPER_OT_7
            );

    //PC0 - PC3 to be inputs
    GPIOC->MODER &= ~(0x000000ff);

    //PC0 - PC3 to be pulled high
    GPIOC->PUPDR &= ~(0x000000ff);
    GPIOC->PUPDR |= (
            GPIO_PUPDR_PUPDR0_0 |
            GPIO_PUPDR_PUPDR1_0 |
            GPIO_PUPDR_PUPDR2_0 |
            GPIO_PUPDR_PUPDR3_0
            );

    return;
}

//============================================================================
// setup_dma()
/*
Enables the RCC clock to the DMA controller
Turn off the enable bit for the channel.
Set CPAR to the address of the GPIOB_ODR register.
Set CMAR to the msg array base address
Set CNDTR to 8
Set the DIRection for copying from-memory-to-peripheral.
Set the MINC to increment the CMAR for every transfer.
Set the memory datum size to 16-bit.
Set the peripheral datum size to 16-bit.
Set the channel for CIRCular operation.
 */
//============================================================================
void setup_dma(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5->CCR &= ~DMA_CCR_EN;

    DMA1_Channel5->CPAR = 0x48000414;

    DMA1_Channel5->CMAR = (uint32_t) msg;

    DMA1_Channel5->CNDTR = 8;

    DMA1_Channel5->CCR |= DMA_CCR_DIR;

    DMA1_Channel5->CCR |= DMA_CCR_MINC;

    DMA1_Channel5->CCR &= ~(DMA_CCR_PINC); //says in debugging portion

    DMA1_Channel5->CCR &= ~(DMA_CCR_MSIZE);
    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;

    DMA1_Channel5->CCR &= ~(DMA_CCR_PSIZE);
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;

    DMA1_Channel5->CCR |= DMA_CCR_CIRC;

    NVIC->ISER[0] = 1 << 11; //dma_ch4_5_6_7 position

    return;
}

//============================================================================
// enable_dma()
// Turn on enable bit?
//============================================================================
void enable_dma(void)
{
    DMA1_Channel5->CCR |= DMA_CCR_EN;
    return;
}

//============================================================================
// init_tim15()
//============================================================================
void init_tim15(void)
{
    RCC->APB2ENR |= (1 << 16);
    TIM15->PSC = (4800 - 1);
    TIM15->ARR = (10 - 1);
    TIM15->DIER |= (1 << 8); //ude bit
    TIM15->CR1 |= 1; //tim_15_en

    return;
}

//=============================================================================
// Part 2: Debounced keypad scanning.
//=============================================================================

uint8_t col; // the column being scanned

void drive_column(int);   // energize one of the column outputs
int  read_rows();         // read the four row inputs
void update_history(int col, int rows); // record the buttons of the driven column
char get_key_event(void); // wait for a button event (press or release)
char get_keypress(void);  // wait for only a button press event.
float getfloat(void);     // read a floating-point number from keypad
void show_keys(void);     // demonstrate get_key_event()

//============================================================================
// The Timer 7 ISR
/*
    // Remember to acknowledge the interrupt here!
    int rows = read_rows();
    update_history(col, rows);
    col = (col + 1) & 3;
    drive_column(col);
 */
//============================================================================

// Write the Timer 7 ISR here.  Be sure to give it the right name.
void TIM7_IRQHandler (void) {

    TIM7->SR &= ~(TIM_SR_UIF);

    int rows = read_rows();
    update_history(col, rows);
    col = (col + 1) & 3;
    drive_column(col);

    return;
}


//============================================================================
// init_tim7()
//1 khz
//============================================================================
void init_tim7(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 4800 - 1;
    TIM7->ARR = 10 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] |= (1 << TIM7_IRQn);
    TIM7->CR1 |= TIM_CR1_CEN;
}

//=============================================================================
// Part 3: Analog-to-digital conversion for a volume level.
//=============================================================================
int volume = 2400;

//============================================================================
// setup_adc()
/*
Enable the clock to GPIO Port A
Set the configuration for analog operation only for the appropriate pins
Enable the clock to the ADC peripheral
Turn on the "high-speed internal" 14 MHz clock (HSI14)

Wait for the 14 MHz clock to be ready
Enable the ADC by setting the ADEN bit in the CR register
Wait for the ADC to be ready
Select the corresponding channel for ADC_IN1 in the CHSELR
Wait for the ADC to be ready
 */
//============================================================================
void setup_adc(void)
{
    //adc_in1 == PA1
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x0000000c; //1100 for pa1 as analog operation
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CR2 |= RCC_CR2_HSI14ON;

    while (!(RCC->CR2 & RCC_CR2_HSI14RDY));
    ADC1->CR |= ADC_CR_ADEN;

    while (!(ADC1->ISR & ADC_ISR_ADRDY));

    //CHSELR
    ADC1->CHSELR = ADC_CHSELR_CHSEL1;

    //wait for the adc to be ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    return;
}

//============================================================================
// Varables for boxcar averaging.
//============================================================================
#define BCSIZE 32
int bcsum = 0;
int boxcar[BCSIZE];
int bcn = 0;
//============================================================================
// Timer 2 ISR
/*
 * Acknowledge the interrupt.
Start the ADC by turning on the ADSTART bit in the CR.
Wait until the EOC bit is set in the ISR.
Implement boxcar averaging using the following code:
    bcsum -= boxcar[bcn];
    bcsum += boxcar[bcn] = ADC1->DR;
    bcn += 1;
    if (bcn >= BCSIZE)
        bcn = 0;
    volume = bcsum / BCSIZE;

 */
//============================================================================

// Write the Timer 2 ISR here.  Be sure to give it the right name.
void TIM2_IRQHandler(void) {

    TIM2->SR &= ~(TIM_SR_UIF);

    ADC1->CR |= ADC_CR_ADSTART;

    while (!(ADC1->ISR & ADC_ISR_EOC));

    bcsum -= boxcar[bcn];
    bcsum += boxcar[bcn] = ADC1->DR;
    bcn += 1;
    if (bcn >= BCSIZE)
        bcn = 0;
    volume = bcsum / BCSIZE;

    for(int x=0; x<10000; x++);

    return;
}

//============================================================================
// init_tim2()
// 10 times per second = 10 HZ
//============================================================================
void init_tim2(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 4800 - 1;
    TIM2->ARR = 1000 - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] |= (1 << TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 3);
    TIM2->CR1 |= TIM_CR1_CEN;
}


//===========================================================================
// Part 4: Create an analog sine wave of a specified frequency
//===========================================================================
void dialer(void);

// Parameters for the wavetable size and expected synthesis rate.
#define N 1000
#define RATE 20000
short int wavetable[N];
int step0 = 0;
int offset0 = 0;
int step1 = 0;
int offset1 = 0;

//===========================================================================
// init_wavetable()
// Write the pattern for a complete cycle of a sine wave into the
// wavetable[] array.
//===========================================================================
void init_wavetable(void)
{
    for(int i=0; i < N; i++)
        wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

//============================================================================
// set_freq()
//============================================================================
void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
}

//============================================================================
// setup_dac()
/*
Enable the RCC clock for the DAC
Select a TIM6 TRGO trigger for the DAC with the
        TSEL field of the CR register
Enable the trigger for the DAC
Enable the DAC
 */
//============================================================================
void setup_dac(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~(DAC_CR_TSEL1_0 |
                 DAC_CR_TSEL1_1 |
                 DAC_CR_TSEL1_2); //tim6 == 000
    //DAC->CR |= DAC_CR_TEN1;
    DAC->CR |= DAC_CR_EN1;
}

//============================================================================
// Timer 6 ISR
/*
// Acknowledge the interrupt right here!
offset0 += step0;
offset1 += step1;
if (offset0 >= (N << 16))
    offset0 -= (N << 16);
if (offset1 >= (N << 16))
    offset1 -= (N << 16);
int samp = wavetable[offset0>>16] + wavetable[offset1>>16];
samp = samp * volume;
samp = samp >> 17;
samp += 2048;
DAC->DHR12R1 = samp;
 */
//============================================================================

// Write the Timer 6 ISR here.  Be sure to give it the right name.
void TIM6_DAC_IRQHandler (void) {
    TIM6->SR &= ~(TIM_SR_UIF);

    offset0 += step0;
    offset1 += step1;
    if (offset0 >= (N << 16))
        offset0 -= (N << 16);
    if (offset1 >= (N << 16))
        offset1 -= (N << 16);
    int samp = wavetable[offset0>>16] + wavetable[offset1>>16];
    samp = samp * volume;
    samp = samp >> 18;
    samp += 1200;
    samp &= 0xfff;
    TIM1->CCR4 = samp;
    //DAC->DHR12R1 = samp;
    return;
}

//============================================================================
// init_tim6()
//============================================================================
void init_tim6(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    //use rate to configure psc/arr
    //arr / psc == RATE
    //1 / RATE == PSC / ARR
    TIM6->PSC = 480 - 1;
    TIM6->ARR = 5 - 1;

    TIM6->CR2 |= TIM_CR2_MMS_1;

    TIM6->DIER |= TIM_DIER_UIE;

    NVIC->ISER[0] |= (1 << TIM6_DAC_IRQn);

    TIM6->CR1 |= TIM_CR1_CEN;
}

//setup_tim3
/*
Configure PC6 - PC9 to be the outputs of Timer 3 channels 1 - 4. These are the four LEDs on your development board.
Enable the RCC clock for Timer 3.
Configure Timer 3 prescaler to divide by 48000.

Configure Timer 3 for PWM mode 1 so that each
channel output can have a CCR value between 0 and 1000:
Configure the ARR so that the timer update rate is 1 Hz.
Set each of the OCxM fields of the TIM3_CCMR1 and
TIM3_CCMR2 registers to set up PWM mode 1.

Enable the four channel outputs in the TIM3_CCER register.
Enable the Timer 3 counter.


Set each the Timer 3 CCRx registers as follows:
TIM3_CCR1 = 800
TIM3_CCR2 = 400
TIM3_CCR3 = 200
TIM3_CCR4 = 100
 */

void setup_tim3(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;


    GPIOC->MODER &= ~(
            GPIO_MODER_MODER6 |
            GPIO_MODER_MODER7 |
            GPIO_MODER_MODER8 |
            GPIO_MODER_MODER9
    );

    GPIOC->MODER |= (
            GPIO_MODER_MODER6_1 |
            GPIO_MODER_MODER7_1 |
            GPIO_MODER_MODER8_1 |
            GPIO_MODER_MODER9_1
    );

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 480 - 1;
    TIM3->ARR = 1000 - 1;

    //setting to pwm mode 1
    TIM3->CCMR1 &= ~TIM_CCMR1_OC1M; //6
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

    TIM3->CCMR1 &= ~TIM_CCMR1_OC2M; //7
    TIM3->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

    TIM3->CCMR2 &= ~TIM_CCMR2_OC3M_2; //8
    TIM3->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;

    TIM3->CCMR2 &= ~TIM_CCMR2_OC4M_2; //9
    TIM3->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;

    TIM3->CCER |= (
            TIM_CCER_CC1E |
            TIM_CCER_CC2E |
            TIM_CCER_CC3E |
            TIM_CCER_CC4E
    );

    TIM3->CR1 |= TIM_CR1_CEN;

    TIM3->CCR1 = 800;
    TIM3->CCR2 = 400;
    TIM3->CCR3 = 200;
    TIM3->CCR4 = 100;

    return;
}

void setup_tim1(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    //TIM1_CH1: PA8
    //TIM1_CH2: PA9
    //TIM1_CH3: PA10
    //TIM1_CH4: PA11
    //set to alternate form (10)


    GPIOA->MODER &= ~(
            GPIO_MODER_MODER8 |
            GPIO_MODER_MODER9 |
            GPIO_MODER_MODER10 |
            GPIO_MODER_MODER11
    );

    GPIOA->MODER |= (
            GPIO_MODER_MODER8_1 |
            GPIO_MODER_MODER9_1 |
            GPIO_MODER_MODER10_1 |
            GPIO_MODER_MODER11_1
    );

    GPIOA->AFR[1] &= ~0xffff; //set 8,9,10,11 to 0
    GPIOA->AFR[1] |= 0x2222; //set to af2

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->BDTR |= TIM_BDTR_MOE;

    TIM1->PSC = 2 - 1;
    TIM1->ARR = 2400 - 1;

    //setting to pwm mode 1
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

    TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

    TIM1->CCMR2 &= ~TIM_CCMR2_OC3M_2;
    TIM1->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;

    TIM1->CCMR2 &= ~TIM_CCMR2_OC4M_2;
    TIM1->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;

    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;

    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

    TIM1->CR1 |= TIM_CR1_CEN;

    return;
}

int getrgb(void);

void setrgb(int rgb)
{
    //rgb = 0x112599
    //red = 11% = TIM_CCR1
    //green = 25% = TIM_CCR2
    //blue = 99% = TIM_CCR3
    int red, blue, green, red2, blue2, green2, red3, blue3, green3;

    red = rgb >> (4 * 5) & (0xf); //tens place
    red2 = rgb >> (4 * 4) & (0xf); //ones place
    red3 = (red << 4) | red2; //tens and ones place combined

    green = rgb >> (4 * 3) & (0xf); //tens place
    green2 = rgb >> (4 * 2) & (0xf); //ones place
    green3 = (green << 4) | green2;

    blue = rgb >> (4 * 1) & (0xf); //tens place
    blue2 = rgb >> (4 * 0) & (0xf); //ones place
    blue3 = (blue << 4) | blue2;

    red3 = (red2 * 10) + (red);
    green3 = (green2 * 10) + green;
    blue3 = (blue2 * 10) + blue;


    //arr = 2400
    TIM1->CCR1 = 24 * (100 - red3);
    TIM1->CCR2 = 24 * (100 - green3);
    TIM1->CCR3 = 24 * (100 - blue3);


    return;

}

//============================================================================
// All the things you need to test your subroutines.
//============================================================================
int main(void)
{

    // Demonstrate part 1
//#define TEST_TIMER3
#ifdef TEST_TIMER3
    setup_tim3();
    for(;;) { }
#endif

    // Initialize the display to something interesting to get started.
    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];

    enable_ports();
    setup_dma();
    enable_dma();
    init_tim15();
    init_tim7();
    setup_adc();
    init_tim2();
    init_wavetable();
    init_tim6();

    setup_tim1();

    // demonstrate part 2
//#define TEST_TIM1
#ifdef TEST_TIM1
    for(;;) {
        for(float x=10; x<2400; x *= 1.1) {
            TIM1->CCR1 = TIM1->CCR2 = TIM1->CCR3 = 2400-x;
            nano_wait(100000000);
        }
    }
#endif

    // demonstrate part 3
//#define MIX_TONES
#ifdef MIX_TONES
    for(;;) {
        char key = get_keypress();
        if (key == 'A')
            set_freq(0,getfloat());
        if (key == 'B')
            set_freq(1,getfloat());
    }
#endif

    // demonstrate part 4
#define TEST_SETRGB
#ifdef TEST_SETRGB
    for(;;) {
        char key = get_keypress();
        if (key == 'A')
            set_freq(0,getfloat());
        if (key == 'B')
            set_freq(1,getfloat());
        if (key == 'D')
            setrgb(getrgb());
    }
#endif

    // Have fun.
    dialer();
}
