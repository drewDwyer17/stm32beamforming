#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "src/PhaseShifter.c"
#include <stdint.h>

//for our stm32 family
//alternate function (AF) pins supported for each of the lines we're mapping from the MCU to the Phaseshifter:
// PA5 = SCK
// PA6 = MISO
// PA7 = MOSI

//part 1

#define SPI2_PS_LE_PORT GPIOB//treat the LE signal as the chip select 
#define SPI2_PS_LE_PIN GPIO12 

#define SPI2_PS_SP_PORT GPIOC
#define SPI2_PS_SP_PIN GPIO10

#define SPI2_PS_MISO_PORT GPIOC
#define SPI2_PS_MISO_PIN GPIO12//(SD01, d6 on PS side)

#define SPI2_PS_CLK_PORT GPIOB
#define SPI2_PS_CLK_PIN GPIO13

#define SPI2_PS_MOSI_PORT GPIOB
#define SPI2_PS_MOSI_PIN GPIO15


// #define SPI2_PS_PIN //what pin? The CPOL (clock polarity) bit controls the idle state value of 
//the clock when no data is being transferred. If 
//CPOL is reset, the SCK pin has a low-level idle state. If set, high 
//this configuration is used when MCU is master, NSS signal driven manually with GPIO

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

    //set CS high to prevent data transfer
    gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);

    gpio_set_af(SPI2_PS_CLK_PORT, GPIO_AF0, SPI2_PS_CLK_PIN); 
    gpio_set_af(SPI2_PS_MOSI_PORT, GPIO_AF0, SPI2_PS_MOSI_PIN); 
    gpio_set_af(SPI2_PS_MISO_PORT, GPIO_AF0, SPI2_PS_MISO_PIN); 

    //SDO1 phase shifter output is D6 of the PS schematic, need to map to MISO. D6 is mapped to PC12 MCU side
    // // now set and enable SPI2 to use master mode and 
    //set the idle state of the clock Low for CPOL = 0 ( TRM "The idle state of SCK must correspond to the polarity selected in the SPIx_CR1 register (by 
    // pulling up SCK if CPOL=1 or pulling down SCK if CPOL=0"

    //Note april 5 think i need to change the CPHA to 0 . 

    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 0, SPI_CR1_LSBFIRST); //make sure to capture all of the recieve bits 
    
}

int main(void)
{
    
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 

    pe448spisetup();

    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low before sending the clock to prevent tx" 
                                       

    spi_enable(SPI2);
    
    double requestedShift_deg = 201.2;
    bool optBit = 0;
    uint8_t unitAddressWord = 0b1100; //address of the unit we're trying to control, 4 bits for up to 16 units.
    volatile uint16_t *response = NULL; 
    

    while (1)
    {
        volatile uint16_t command = CreatePhaseRequestWithMMasks(requestedShift_deg, optBit, unitAddressWord);
        //send the frame
        //1001101000011
        
        //read back the received word to clear RXNE / capture response
        volatile uint16_t phaseShifterResponse =spi_xfer(SPI2, command);
        *response = phaseShifterResponse;

        break; //break after one command for now, just to test the communication.
        // gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); // set CS high to end the transmission, LE = latch enable. 

    }
     

    return 0;
}