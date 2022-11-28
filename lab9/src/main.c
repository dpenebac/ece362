#include "stm32f0xx.h"
#include <string.h> // for memcpy() declaration

void nano_wait(unsigned int);
extern const char font[128];

//===========================================================================
// Debouncing a Keypad
//===========================================================================

void drive_column(int);
int read_rows();
void update_history(col, rows);

uint8_t col;
//===========================================================================
// Configure timer 7 to invoke the update interrupt at 1kHz
// Copy from lab 6 or 7.
//===========================================================================
void init_tim7(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 4800 - 1;
    TIM7->ARR = 10 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] |= (1 << TIM7_IRQn);
    TIM7->CR1 |= TIM_CR1_CEN;
}

//===========================================================================
// Copy the Timer 7 ISR from lab 7
//===========================================================================
// Write the Timer 7 ISR here.  Be sure to give it the right name.
void TIM7_IRQHandler (void) {

    TIM7->SR &= ~(TIM_SR_UIF);

    int rows = read_rows();
    update_history(col, rows);
    col = (col + 1) & 3;
    drive_column(col);

    return;
}

//===========================================================================
// SPI DMA LED Array
//===========================================================================
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };

void setup_dma(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5->CCR &= ~DMA_CCR_EN;

    DMA1_Channel5->CPAR = 0x4000380c; //spi2->dr 0x0c spi2 offset 4000 3800

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

void enable_dma(void)
{
    DMA1_Channel5->CCR |= DMA_CCR_EN;
    return;
}

void init_spi2(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER &= ~(
            GPIO_MODER_MODER12 |
            GPIO_MODER_MODER13 |
            GPIO_MODER_MODER15
    );

    GPIOB->MODER |= (
            GPIO_MODER_MODER12_1 |
            GPIO_MODER_MODER13_1 |
            GPIO_MODER_MODER15_1
    );

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    SPI2->CR1 &= ~(SPI_CR1_SPE);
    SPI2->CR1 |= (7 << 3); //set br to be 111
    SPI2->CR2 = 0x0f00; //configure interface for 16-bit word size
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR2 |= SPI_CR2_SSOE;
    SPI2->CR2 |= SPI_CR2_NSSP;
    SPI2->CR2 |= SPI_CR2_TXDMAEN;
    SPI2->CR1 |= SPI_CR1_SPE;
}

void setup_spi2_dma(void) {
    setup_dma();
    SPI2->CR2 |= SPI_CR2_TXDMAEN;// Transfer register empty DMA enable
}

void enable_spi2_dma(void) {
    enable_dma();
}

//===========================================================================
// 2.1 Initialize I2C
//===========================================================================
#define GPIOEX_ADDR 0x20  // ENTER GPIO EXPANDER I2C ADDRESS HERE
#define EEPROM_ADDR 0x50  // ENTER EEPROM I2C ADDRESS HERE

/*
Write a C subroutine named init_i2c() that configures PB6 and PB7 to be, respectively,
SCL and SDA of the I2C1 peripheral.
Enable the RCC clock to GPIO Port B and the I2C1 channel,
set the MODER fields for PB6 and PB7,
and set the alternate function register entries.

Then configure the I2C1 channel as follows:
First, disable the PE bit in CR1 before making the following configuration changes.
Turn off the ANFOFF bit (turn on the analog noise filter).
Disable the error interrupt.
Turn off the NOSTRETCH bit in CR1 to enable clock stretching.
Set the TIMINGR register as follows: (Note that these are configurations found on Table 83 of the Family Reference Manual. Why is I2C1's clock 8 MHz? See Figure 209 of the Reference Manual for a hint).
Set the prescaler to 0.
Set the SCLDEL field to 3.
Set the SDADEL field to 1.
Set the SCLH field to 3.
Set the SCLL field to 9.
Disable both of the "own addresses", OAR1 and OAR2.
Configure the ADD10 field of CR2 for 7-bit mode.
Turn on the AUTOEND setting to enable automatic end.
Enable the channel by setting the PE bit in CR1.
 */

void init_i2c(void) {

    /*
    Write a C subroutine named init_i2c() that configures PB6 and PB7 to be, respectively,
    SCL and SDA of the I2C1 peripheral.

    Enable the RCC clock to GPIO Port B and the I2C1 channel,
    set the MODER fields for PB6 and PB7,
    and set the alternate function register entries.
    */

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~(
            GPIO_MODER_MODER6 |
            GPIO_MODER_MODER7
    );

    GPIOB->MODER |= (
            GPIO_MODER_MODER6_1 |
            GPIO_MODER_MODER7_1
    );

    GPIOB->AFR[0] &= ~(
            GPIO_AFRL_AFR6 |
            GPIO_AFRL_AFR7
    );

    GPIOB->AFR[0] |= (
            (1 << (4 * 6)) |
            (1 << (4 * 7))
    ); //setting 6 and 7 to be af1

    /*
    Then configure the I2C1 channel as follows:
    First, disable the PE bit in CR1 before making the following configuration changes.
    Turn off the ANFOFF pubit (turn on the analog noise filter).
    Disable the error interrupt.
    Turn off the NOSTRETCH bit in CR1 to enable clock stretching.

    Set the TIMINGR register as follows: (Note that these are configurations found on Table 83 of the Family Reference Manual. Why is I2C1's clock 8 MHz? See Figure 209 of the Reference Manual for a hint).
    Set the prescaler to 0.
    Set the SCLDEL field to 3.
    Set the SDADEL field to 1.
    Set the SCLH field to 3.
    Set the SCLL field to 9.

    Disable both of the "own addresses", OAR1 and OAR2.

    Configure the ADD10 field of CR2 for 7-bit mode.
    Turn on the AUTOEND setting to enable automatic end.
    Enable the channel by setting the PE bit in CR1.
     */

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;
    I2C1->CR1 &= ~I2C_CR1_ERRIE;
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC; //clear prescaler
    I2C1->TIMINGR |= 0 << 28; //set prescaler to 0
    I2C1->TIMINGR |= 3 << 20;
    I2C1->TIMINGR |= 1 << 16;
    I2C1->TIMINGR |= 3 << 8;
    I2C1->TIMINGR |= 9 << 0;

    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

    //I2C1->OAR1 = I2C_OAR1_OA1EN | 0x2;

    I2C1->CR2 &= ~I2C_CR2_ADD10; //set to 0 for 7 bit mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR1 |= I2C_CR1_PE;

    return;
}


//===========================================================================
// 2.2 I2C helpers
//===========================================================================

void i2c_waitidle(void) {
    while ((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
    return;
}

void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir) {
    uint32_t tmpreg = I2C1->CR2;

    tmpreg &= ~(
            I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
            I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START |
            I2C_CR2_STOP
    );

    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN; //read from slave
    else
        tmpreg &= ~(I2C_CR2_RD_WRN); //write to slave

    tmpreg |= ((devaddr << 1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;

    return;
}

void i2c_stop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF) {
        return;
    }

    I2C1->CR2 |= I2C_CR2_STOP;

    while ((I2C1->ISR & I2C_ISR_STOPF) == 0);

    I2C1->ICR |= I2C_ICR_STOPCF;

    return;
}

int i2c_checknack(void) {
    if ((I2C1->ISR & I2C_ISR_NACKF) == I2C_ISR_NACKF){
        return 1;
    }

    return 0;
}

void i2c_clearnack(void) {
    I2C1->ICR |= I2C_ICR_NACKCF;
}

int i2c_senddata(uint8_t devaddr, const void *data, uint8_t size) {
    int i;

    if (size <= 0 || data == 0) {
        return -1;
    }

    uint8_t * udata = (uint8_t*) data;
    i2c_waitidle();
    i2c_start(devaddr, size, 0);

    for (i = 0; i < size; i++) {

        int count = 0;
        while ((I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000) return -1;
            if (i2c_checknack()) { i2c_clearnack(); i2c_stop(); return -1; }
        }

        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    while ((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ((I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;

    i2c_stop();

    return 0;
}

int i2c_recvdata(uint8_t devaddr, void *data, uint8_t size) {

    int i;
    if (size <= 0 || data == 0) return -1;

    uint8_t *udata = (uint8_t*) data;

    i2c_waitidle();

    i2c_start(devaddr, size, 1);

    for (i = 0; i < size; i++) {
        int count = 0;

        while ((I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000) return -1;
            if (i2c_checknack()) { i2c_clearnack(); i2c_stop(); return -1; }
        }

        udata[i] = I2C1->RXDR;
    }

    while ((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ((I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;

    i2c_stop();

    return 0;

}


//===========================================================================
// 2.4 GPIO Expander
//===========================================================================
void gpio_write(uint8_t reg, uint8_t val) {
    //To do this, set up a two-byte array with the two parameters and use the i2c_senddata() subroutine to send them to the MCP23008.
    uint8_t data[2] = { reg, val };
    i2c_senddata(0x20, data, 2);
}

uint8_t gpio_read(uint8_t reg) {
    //Return this value. To do this, set up a one-byte array with the register number, send it with i2c_senddata().
    //Reuse the one-byte array to read one byte from the MCP23008 with i2c_recvdata().

    uint8_t data[1] = { reg };
    i2c_senddata(0x20, data, 1);
    i2c_recvdata(0x20, data, 1);

    return data[0];
}

/*
Write a C subroutine named init_expander() that uses void gpio_write(uint8_t reg, uint8_t value)
to configure the GPIO Expander.
It should configure the IODIR register so that GP0-3 are outputs and GP4-7 are inputs.
It should also turn on the internal pull ups on pins GP4-7 and reverse their polarity.
 */

void init_expander(void) {
    uint8_t reg = 0x0;
    uint8_t value = 0x0;

    //It should configure the IODIR register so that GP0-3 are outputs and GP4-7 are inputs.
    reg = 0x00; //iodir reg
    value = 0xf0; //1111 0000
    gpio_write(reg, value);

    //It should also turn on the internal pull ups on pins GP4-7 and reverse their polarity.
    reg = 0x06;
    value = 0xf0;
    gpio_write(reg, value);

    //reverse their polarity.
    reg = 0x01;
    value = 0xf0;
    gpio_write(reg, value);

    return;
}

void drive_column(int c) {
    gpio_write(10, ~(1 << (3 - c)) );
}

int read_rows() {
    uint8_t data = gpio_read(9);
    data &= 0xf0;
    for (int i = 0; i < 4; i++) {
        uint8_t bit = data & (1 << (4 + i));
        bit >>= (4 + i);
        bit <<= (3 - i);
        data |= bit;
    }
    return data & 0xf;
}


//===========================================================================
// 2.4 EEPROM functions
//===========================================================================
void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    //int i2c_senddata(uint8_t devaddr, const void *data, uint8_t size)
    //i2c_senddata(EEPROM_ADDR, data, len);

    /*
    Since writes will be no longer than 32 bytes, we recommend you create an array of 34 bytes in this subroutine.
    In the first two bytes, put the decomposed 12-bit address.
    Copy the rest of the data into bytes 2 through 33.
    Invoke i2c_senddata() with a length that is the data size plus 2.
     */

    //loc == 0x0234
    //tmp[0] = 02
    //tmp[1] = 34

    char tmp[34];
    tmp[0] = (loc >> 8) & 0x0f;
    tmp[1] = loc & 0xff;

    int i;
    for (i = 2; i < 33; i++) {
        tmp[i] = data[i - 2];
    }

    i2c_senddata(EEPROM_ADDR, tmp, len + 2);

    return;

}

/*
Wait for the I2C channel to be idle.
Initiate an i2c_start() with the correct I2C EEPROM device ID, zero length, and write-intent.
Wait until either the TC flag or NACKF flag is set.
If the NACKF flag is set, clear it, invoke i2c_stop() and return 0.
If the NACKF flag is not set, invoke i2c_stop() and return 1.
 */

int eeprom_write_complete(void) {
    i2c_waitidle();

    i2c_start(EEPROM_ADDR,0,0); //0 is to write

    while ((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if (i2c_checknack()) {
        i2c_clearnack();
        i2c_stop();
        return 0;
    }
    else {
        i2c_stop();
        return 1;
    }
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    TIM7->CR1 &= ~TIM_CR1_CEN; // Pause keypad scanning.

    // ... your code here
    char tmp[34];
    tmp[0] = (loc >> 8) & 0x0f;
    tmp[1] = loc & 0xff;

    i2c_senddata(EEPROM_ADDR,tmp,2);
    i2c_recvdata(EEPROM_ADDR,data,len);

    TIM7->CR1 |= TIM_CR1_CEN; // Resume keypad scanning.

    return;
}


void eeprom_blocking_write(uint16_t loc, const char* data, uint8_t len) {
    TIM7->CR1 &= ~TIM_CR1_CEN; // Pause keypad scanning.
    eeprom_write(loc, data, len);
    while (!eeprom_write_complete()); //never finishes
    TIM7->CR1 |= TIM_CR1_CEN; // Resume keypad scanning.
}

//===========================================================================
// Main and supporting functions
//===========================================================================

void serial_ui(void);
void show_keys(void);

int main(void)
{
    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];


    // LED array SPI
    setup_spi2_dma();
    enable_spi2_dma();
    init_spi2();

    // This LAB

    // 2.1 Initialize I2C
    init_i2c();

    // 2.2 Example code for testing
//#define STEP22
#if defined(STEP22)
    for(;;) {
        i2c_waitidle();
        i2c_start(GPIOEX_ADDR,0,0);
        i2c_clearnack();
        i2c_stop();
    }
#endif

    // 2.3 Example code for testing
//#define STEP2.3
#if defined(STEP23)
    for(;;) {
        uint8_t data[2] = { 0x00, 0xff };
        i2c_senddata(0x20, data, 2);
    }
#endif

//#define STEP23
#if defined(STEP23)
    for(;;) {
        uint8_t data[2] = { 0x00, 0x00 };
        i2c_senddata(0x20, data, 1); // Select IODIR register
        i2c_recvdata(0x20, data, 1);
    }
#endif

    // 2.4 Expander setup
    init_expander();
    init_tim7();

    // 2.5 Interface for reading/writing memory.
    serial_ui();

    show_keys();
}