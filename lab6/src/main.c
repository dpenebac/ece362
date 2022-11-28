#include "stm32f0xx.h"
#include <math.h>   // for M_PI

void nano_wait(int);

//=============================================================================
// Part 1: 7-segment display update with DMA
//=============================================================================

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
uint32_t volume = 2048;

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
    DAC->CR |= DAC_CR_TEN1;
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
    samp = samp >> 17;
    samp += 2048;
    DAC->DHR12R1 = samp;
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

//============================================================================
// All the things you need to test your subroutines.
//============================================================================
int main(void)
{
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

    // Demonstrate part 1
//#define SCROLL_DISPLAY
#ifdef SCROLL_DISPLAY
    for(;;)
        for(int i=0; i<8; i++) {
            print(&"Hello...Hello..."[i]);
            nano_wait(250000000);
        }
#endif

    init_tim7();

    // Demonstrate part 2
//#define SHOW_KEY_EVENTS
#ifdef SHOW_KEY_EVENTS
    for (;;) {
        show_keys();
    }
#endif

    setup_adc();
    init_tim2();

    // Demonstrate part 3
//#define SHOW_VOLTAGE
#ifdef SHOW_VOLTAGE
    for(;;) {
        printfloat(2.95 * volume / 4096);
    }
#endif

    init_wavetable();
    setup_dac();
    init_tim6();

//#define ONE_TONE
#ifdef ONE_TONE
    for(;;) {
        float f = getfloat();
        set_freq(0,f);
    }
#endif

    // demonstrate part 4
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

    // Have fun.
    dialer();
}
