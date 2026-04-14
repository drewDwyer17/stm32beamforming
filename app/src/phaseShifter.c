#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PE48820B_NUMSTATES 256u
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG)


// Easier way of doing it would just be  (the most simple working method) : 
static uint16_t CreatePhaseRequest(double requestedShift_deg, bool optBit, uint8_t unitAddressWord)
{

    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);
    
    int fullcommand = (phaseSetWord << 5) | (optBit << 4) | unitAddressWord;

    return fullcommand;
} 


static uint16_t CreatePhaseRequestWithMMasks(double requestedShift_deg, bool optBit, uint8_t unitAddressWord)
{
    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);

    uint16_t fullcommand =
        ((phaseSetWord & 0xFFu) << 5) |
        (((uint16_t)optBit & 0x1u) << 4) |
        (unitAddressWord & 0x0Fu);

    return fullcommand;
}


/* 
Unit test: Sending a command via spi to the phase shifter, sample main function 


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "src/PhaseShifter.c"
#include <stdint.h>
#include <libopencm3/stm32/timer.h>

#define SPI2_PS_LE_PORT GPIOB//treat the LE signal as the chip select 
#define SPI2_PS_LE_PIN GPIO12 

#define SPI2_PS_SP_PORT GPIOC
#define SPI2_PS_SP_PIN GPIO10

#define SPI2_PS_MISO_PORT GPIOC
#define SPI2_PS_MISO_PIN GPIO11//(SD01, d6 on PS side)
#define SPI2_PS_CLK_PORT GPIOB
#define SPI2_PS_CLK_PIN GPIO13

#define SPI2_PS_MOSI_PORT GPIOB
#define SPI2_PS_MOSI_PIN GPIO15

void pe448spisetup(void)
{
    //enable the SPI
    rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC); 


    //need to set the PS SP pin HIGH for serial communication 
    gpio_mode_setup(SPI2_PS_SP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_PS_SP_PIN); 
    gpio_set(SPI2_PS_SP_PORT, SPI2_PS_SP_PIN); //set SP high for serial communication 

    //now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI2_PS_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_CLK_PIN); 
    gpio_mode_setup(SPI2_PS_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_MISO_PIN); 
    gpio_mode_setup(SPI2_PS_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_MOSI_PIN); 

    gpio_mode_setup(SPI2_PS_LE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_PS_LE_PIN); //set latch enable (CS) as general output 

    

    gpio_set_af(SPI2_PS_CLK_PORT, GPIO_AF0, SPI2_PS_CLK_PIN); 
    gpio_set_af(SPI2_PS_MOSI_PORT, GPIO_AF0, SPI2_PS_MOSI_PIN); 
    gpio_set_af(SPI2_PS_MISO_PORT, GPIO_AF0, SPI2_PS_MISO_PIN); 
=

    spi_set_data_size(SPI2, SPI_CR2_DS_13BIT); //our command size is 13 bits. // I think we should pad it to be 16 bits. The Ps will filter them out in one example. 
    spi_fifo_reception_threshold_16bit(SPI2);
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 0, SPI_CR1_LSBFIRST); //make sure to capture all of the recieve bits 
    
}

#define PE48820B_NUMSTATES 256u
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG)


int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 

    pe448spisetup();                                   

    spi_enable(SPI2);
    
    // double requestedShift_deg = 205.3;
    // bool optBit = 0;
    // uint8_t unitAddressWord = 0b0011; //address of the unit we're trying to control, 4 bits for up to 16 units.

    volatile uint16_t command = 0b0010000000000; 
    volatile uint16_t phaseShifterResponse = 0;

    // uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);
    
    // command2 = (phaseSetWord << 5) | (optBit << 4) | unitAddressWord;

    while (1)
    {
        gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low
        //send the frame
        //1001101000011
        //read back the received word to clear RXNE / capture response
        spi_send(SPI2, command);
        phaseShifterResponse =spi_read(SPI2);
        // spi_clean_disable(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        //then send another command. 
        spi_send(SPI2, 0b0011111100000); //this shouldn't matter. Based on the datasheet, whatever we send fter LE goes high should be ignored. We can test this by sending
         a different command after the first one and seeing if the response changes.
         #oscilloscope readings confirm
         
        break;

    }
    return 0;
}

*/





























