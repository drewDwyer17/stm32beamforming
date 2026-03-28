#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "src/PhaseShiftMgr.c"
#include <stdint.h>

//for our stm32 family
//alternate function (AF) pins supported for each of the lines we're mapping from the MCU to the Phaseshifter:
// PA5 = SCK
// PA6 = MISO
// PA7 = MOSI

//part 1

#define SPI2_LE_PORT GPIOB//treat the LE signal as the chip select 
#define SPI2_LE_PIN GPIO12 

#define SPI2_SP_PORT GPIOC
#define SPI2_SP_PIN GPIO10

#define SPI2_MISO_PORT GPIOC
#define SPI2_MISO_PIN GPIO12//(SD01, d6 on PS side)

#define SPI2_CLK_PORT GPIOB
#define SPI2_CLK_PIN GPIO13

#define SPI2_MOSI_PORT GPIOB
#define SPI2_MOSI_PIN GPIO15


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
    gpio_mode_setup(SPI2_SP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_SP_PIN); 
    gpio_set(SPI2_SP_PORT, SPI2_SP_PIN); //set SP high for serial communication 

    //now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI2_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_CLK_PIN); 
    gpio_mode_setup(SPI2_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_MISO_PIN); 
    gpio_mode_setup(SPI2_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_MOSI_PIN); 

    gpio_mode_setup(SPI2_LE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_LE_PIN); //set latch enable (CS) as general output 

    //set CS high to prevent data transfer during configuration
    gpio_set(SPI2_LE_PORT, SPI2_LE_PIN);

    gpio_set_af(SPI2_CLK_PORT, GPIO_AF0, SPI2_CLK_PIN); 
    gpio_set_af(SPI2_MOSI_PORT, GPIO_AF0, SPI2_MOSI_PIN); 
    gpio_set_af(SPI2_MISO_PORT, GPIO_AF0, SPI2_MISO_PIN); 

    //SDO1 phase shifter output is D6 of the PS schematic, need to map to MISO. D6 is mapped to PC12 MCU side
    // // now set and enable SPI2 to use master mode and 
    //set the idle state of the clock Low for CPOL = 0 ( TRM "The idle state of SCK must correspond to the polarity selected in the SPIx_CR1 register (by 
    // pulling up SCK if CPOL=1 or pulling down SCK if CPOL=0"
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 1, SPI_CR1_LSBFIRST); 
    spi_set_data_size(SPI2, SPI_CR2_DS_13BIT); //our command size is 13 bits. 
    spi_fifo_reception_threshold_16bit(SPI2); //make sure to capture all of the recieve bits 
    
}

int main(void)
{
    
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    volatile PhaseShiftRequest_t req;
    InitNewPhaseShiftRequest(&req); 

    pe448spisetup();

    gpio_clear(SPI2_LE_PORT, SPI2_LE_PIN); //SET LOW FOR TX " TRM It is recommended to enable the SPI slave before the master sends the clock. If not, 
                                        // undesired data transmission might occur."

    spi_enable(SPI2);
    
    double testPhaseShift = 205.3;
    bool optBit = 0;
    uint8_t unitAddr = 0b1100;   // = 3

    SetPhaseShiftRequest(205.3, 0, 0b1100, &req);

    while (1)
    {


        // if (req.rc != Err_Ok) continue; //builds a command and gives it to the req->packedPhaseShiftCmd structure

        uint16_t frame; 

        frame = req.packedPhaseShiftCmd.packedCmdToSend;

        //caculate chksm to checklater
        //not yet implemented

        //send the frame
        //1001101000011
        spi_send(SPI2, frame);

        //read back the received word to clear RXNE / capture response
        req.phaseShifterResponse = spi_read(SPI2); //SDO2 data changes on rising edge of CLK and is valid on falling edge of CLK.

        //release chip select
        gpio_set(SPI2_LE_PORT, SPI2_LE_PIN);

        // validate response
        if (req.phaseShifterResponse != frame) {
            req.rc = Err_PSResponse;
            continue;
        }

        // ClearPhaseShiftRequest(&req); 

        //need a debug function to later print the value in req.rc
    }

    return 0;
}