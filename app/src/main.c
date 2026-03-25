#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "SignalMgr.h"
#include "PhaseShiftMgr.h"
#include <libopencm3/stm32/spi.h>

#define SIGNOFLIFEPORT GPIOA
#define SIGNOFLIFEPIN GPIO9   // PA9

#define SPI1_CLK_PORT  GPIOA
#define SPI1_CLK_PIN   GPIO5
#define SPI1_MOSI_PORT GPIOA //? configure gpio mode as AF for spi mosi. map to phase shifter SI (signal in)
#define SPI1_MOSI_PIN  GPIO7
#define SPI1_MISO_PORT GPIOA //configure gpio mode as AF for Spi miso to capture the response from phaseshifter (from SDO2 PE488 side)
#define SPI1_MISO_PIN  GPIO6
#define SPI1_CS_PORT   GPIOB //chipselect, configure as output. Map this to "latch enable" register for the phaes shifter chip?
#define SPI1_CS_PIN    GPIO0

int main(void) 
{
    //still haven't decided which source to clock SP1 from. it can use 
    // a. PCLK2
    // b. SysCLK
    // c. HSI16 
    // d. MSIK

    //do I need to specify APB1ENR or APB2ENR? what speed of bus do we want to use? 
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);    //alternate function pin for SPI CLK
    rcc_periph_clock_enable(RCC_GPIOA);    //alternate function pin for SPI SI
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    //now set the pin modes as outputs and AF as specified above 
    gpio_mode_setup(SPI1_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_CLK_PIN); 
    gpio_mode_setup(SPI1_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_MOSI_PIN); 
    gpio_mode_setup(SPI1_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_MISO_PIN); 
    gpio_mode_setup(SPI1_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS_PIN); 

    gpio_set_af(SPI1_CLK_PORT, GPIO_AF0, SPI1_CLK_PIN);
    gpio_set_af(SPI1_MOSI_PORT, GPIO_AF0, SPI1_MOSI_PIN);
    gpio_set_af(SPI1_MISO_PORT, GPIO_AF0, SPI1_MISO_PIN);

    gpio_set(SPI1_CS_PORT, SPI1_CS_PIN);
    
    // now set and enable SPI1 to use master mode 
    spi_set_master_mode(SPI1);
    spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_256);  // note sure which clk division to use
    spi_set_clock_polarity_0(SPI1);
    spi_set_clock_phase_0(SPI1);
    spi_send_lsb_first(SPI1);
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);
    spi_enable(SPI1); 

    while (1)
    {
        //depending on if we want to transmit on rising or falling edge of signal, change 
        // the transition paraSmeter, need to review the data sheet
        //make sure to send 
        //think that maybe thelock source decision will preconfigure the 
        //timing for the spi
        // spi_set_clock_polarity_0(SPI1);
        // spi_set_clock_phase_0(SPI1);
        // spi_set_standard_mode(SPI1);
        
        //----------------------------------------------------
        // //now configure the command to send using our functions 
        // // now send? 
        // double degreesToShift = 45.0;
        // bool optBit = 0; 
        // uint8_t unitAdress = 3; 
        // PhaseShiftMgr_Error_t rc; 

        // PhaseShiftRequest_t req = InitNewPhaseShiftRequest(err); 
        
        // if (req.rc =! ok) 
        // ResetPhaseShiftRequest(req); 
        // PackedPhaseShiftCmd_t PhaseShiftCommand = BuildPhaseShiftCmd(degreesToShift, optBit, unitAdress, &req); //return the rc of this function to the req structure that we intiailize, but the
        // //en return a 13 bit command as a packed tupe. he packing function should be wihtin this api to make it readable 
        
        // if (req.err = Et)
        // //perform the CRC check on the command. Function for this isnt implemented 

        // //now we're ready to send it to the phaes shifter. 
        // send it byte by byte 

        // spi_send_phase_shift(phaseshiftcmd
        // BuildPhaseShiftCmd(degreesToShift,   
        // spi_send
    }

    return 0;
}