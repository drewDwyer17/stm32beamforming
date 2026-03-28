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
//all SPI1 on same port
#define SPI1_PORT      GPIOA
#define SPI1_CLK_PIN   GPIO5 //clk line
#define SPI1_MISO_PIN  GPIO6 //incoming response
#define SPI1_MOSI_PIN  GPIO7 //phase shift request cmd
#define SPI1_CS_PIN    GPIO0 //nss pin
#define SPI1_PS_PIN //what pin? The CPOL (clock polarity) bit controls the idle state value of 
//the clock when no data is being transferred. If 
//CPOL is reset, the SCK pin has a low-level idle state. If set, high 
//this configuration is used when MCU is master, NSS signal driven manually with GPIO

void pe448spisetup(void)
{
    //enable the SPI
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);

    //set CS high to prevent data transfer during configuration
    gpio_set(SPI1_PORT, SPI1_CS_PIN);

    //need to set the ps pin high for serial communication 

    //now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_CLK_PIN | SPI1_MOSI_PIN | SPI1_MISO_PIN);
    gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS_PIN);

    gpio_set_af(SPI1_PORT, GPIO_AF0, SPI1_CLK_PIN | SPI1_MOSI_PIN | SPI1_MISO_PIN);

        
    gpio_set(SPI1_PORT, SPI1_CS_PIN); //de-select chip to allow transfer. " TRM It is recommended to enable the SPI slave before the master sends the clock. If not, 
                                        // undesired data transmission might occur."

    // // now set and enable SPI1 to use master mode and 
    //set the idle state of the clock Low for CPOL = 0 ( TRM "The idle state of SCK must correspond to the polarity selected in the SPIx_CR1 register (by 
    // pulling up SCK if CPOL=1 or pulling down SCK if CPOL=0"
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_128, 0, 1, SPI_CR1_LSBFIRST); 
    spi_set_data_size(SPI1, SPI_CR2_DS_13BIT); //our command size is 13 bits. 

    spi_fifo_reception_threshold_16bit(SPI1); //make sure to capture all of the recieve bits 
    
}

int main(void)
{
    PhaseShiftRequest_t req = InitNewPhaseShiftRequest();

    pe448spisetup();
    spi_enable(SPI1);

    while (1)
    {
        double testPhaseShift = 205.3;
        bool optBit = 0b0;
        uint8_t unitAddr = 0b0011;   // = 3

        ClearPhaseShiftRequest(&req);
        
        SetPhaseShiftRequest(testPhaseShift, optBit, unitAddr, &req);

        if (req.rc != Err_Ok) continue; //builds a command and gives it to the req->packedPhaseShiftCmd structure

        uint16_t frame; 

        frame = req.packedPhaseShiftCmd.packedCmdToSend;

        //caculate chksm to checklater
        //not yet implemented

        //manual chip select active low
        gpio_clear(SPI1_PORT, SPI1_CS_PIN);

        //send the frame
        spi_send(SPI1, frame);

        //read back the received word to clear RXNE / capture response
        req.phaseShifterResponse = spi_read(SPI1); //SDO2 data changes on rising edge of CLK and is valid on falling edge of CLK.

        //release chip select
        gpio_set(SPI1_PORT, SPI1_CS_PIN);

        // validate response
        if (req.phaseShifterResponse != frame) {
            req.rc = Err_PSResponse;
            continue;
        }

        //need a debug function to later print the value in req.rc
    }

    return 0;
}