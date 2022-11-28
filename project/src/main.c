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

#define EEPROM_ADDR 0x50


//===========================================================================
// 2.2 I2C helpers
//===========================================================================

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
    //TIM7->CR1 &= ~TIM_CR1_CEN; // Pause keypad scanning.

    // ... your code here
    char tmp[34];
    tmp[0] = (loc >> 8) & 0x0f;
    tmp[1] = loc & 0xff;

    i2c_senddata(EEPROM_ADDR,tmp,2);
    i2c_recvdata(EEPROM_ADDR,data,len);

    //TIM7->CR1 |= TIM_CR1_CEN; // Resume keypad scanning.

    return;
}


void eeprom_blocking_write(uint16_t loc, const char* data, uint8_t len) {
    //TIM7->CR1 &= ~TIM_CR1_CEN; // Pause keypad scanning.
    eeprom_write(loc, data, len);
    while (!eeprom_write_complete()); //never finishes
    //TIM7->CR1 |= TIM_CR1_CEN; // Resume keypad scanning.
}

void update_scores(void);
void sort_scores(void);
void format_scores(char []);
void unformat_scores(char []);
void reset_scores(void);

struct score {
    char ID[3];
    char SCORE;
};

/*
void set_score(struct score s, char ID[], uint8_t score) {
    s.ID = ID;
    s.SCORE = score;

    return;
}
*/

struct score highscores[6];

void update_scores(void) {

    char scores[20];

    if (highscores[5].ID[0] == 'R' &&
        highscores[5].ID[1] == 'S' &&
        highscores[5].ID[2] == 'T') { //check if new score has a name of RST
        reset_scores();
    }
    else {
        //sort
        sort_scores(); //sorts current loaded highscores
    }

    //format scores
    format_scores(scores);

    //write
    eeprom_blocking_write((uint16_t)0x00, scores, 20);

    return;
}

void sort_scores(void) {
    int i;
    //newhighscore is set to the 6th high score

    for (i = 5; i > 0; i--) {
        if (highscores[i].SCORE > highscores[i - 1].SCORE) {
            //swap
            struct score temp = highscores[i - 1];
            highscores[i - 1] = highscores[i];
            highscores[i] = temp;
        }
        else {
            break; //if no swaps then finished
        }
    }

    return;
}

void format_scores(char scores[]) { //writes to array using global struct

    //FORMAT
    /*
       ID, SCORE, ID, SCORE
     */

    int i;
    int c;

    c = 0;

    for (i = 0; i < 20; i += 4) {
        scores[i] = highscores[c].ID[0];
        scores[i + 1] = highscores[c].ID[1];
        scores[i + 2] = highscores[c].ID[2];
        scores[i + 3] = highscores[c].SCORE;
        c += 1;
    }

    return;
}

void unformat_scores(char scores[]) { //writes into global struct
    int i;
    int c = 0;

    for (i = 0; i < 20; i += 4) {
        highscores[c].ID[0] = scores[i];
        highscores[c].ID[1] = scores[i + 1];
        highscores[c].ID[2] = scores[i + 2];
        highscores[c].SCORE = scores[i + 3];
        c += 1;
    }
}

void read_scores(void) { //sets scores for global variable stored on eeprom

    //read_scores

    char scores[20];

    eeprom_read((uint16_t)0x00, scores, 20);

    unformat_scores(scores);

    return;
}

void reset_scores(void) {

    highscores[0].ID[0] = 'R'; //largest
    highscores[0].ID[1] = 'I'; //largest
    highscores[0].ID[2] = 'K'; //largest
    highscores[0].SCORE = 17; //largest

    highscores[1].ID[0] = 'T'; //largest
    highscores[1].ID[1] = 'I'; //largest
    highscores[1].ID[2] = 'M'; //largest
    highscores[1].SCORE = 5; //largest

    highscores[2].ID[0] = 'E'; //largest
    highscores[2].ID[1] = 'C'; //largest
    highscores[2].ID[2] = 'E'; //largest
    highscores[2].SCORE = 4; //largest

    highscores[3].ID[0] = 'D'; //largest
    highscores[3].ID[1] = 'A'; //largest
    highscores[3].ID[2] = 'C'; //largest
    highscores[3].SCORE = 3; //largest

    highscores[4].ID[0] = 'S'; //largest
    highscores[4].ID[1] = 'P'; //largest
    highscores[4].ID[2] = 'I'; //largest
    highscores[4].SCORE = 0; //largest

    highscores[5].ID[0] = 'N'; //largest
    highscores[5].ID[1] = 'E'; //largest
    highscores[5].ID[2] = 'W'; //largest
    highscores[5].SCORE = 8; //largest

    return;
}

int main(void)
{

    //to write, call
    //eeprom_blocking_write(uint16_t loc, const char* data, uint8_t len)
    //loc = 0x32 location must be divisible by 32


    /*
    init_i2c();

    reset_scores();
    update_scores();
    read_scores();

    highscores[5].ID[0] = 'D'; //largest
    highscores[5].ID[1] = 'O'; //largest
    highscores[5].ID[2] = 'R'; //largest
    highscores[5].SCORE = 100; //largest

    update_scores();
    read_scores();

    highscores[5].ID[0] = 'R'; //largest
    highscores[5].ID[1] = 'S'; //largest
    highscores[5].ID[2] = 'T'; //largest
    highscores[5].SCORE = 100; //largest

    update_scores();
    read_scores();
    */

    init_i2c();
    read_scores();

    //add new score
    highscores[5].ID[0] = 'D'; //largest
    highscores[5].ID[1] = 'O'; //largest
    highscores[5].ID[2] = 'R'; //largest
    highscores[5].SCORE = 100; //largest

    update_scores();
    read_scores();

    return(0);
}
