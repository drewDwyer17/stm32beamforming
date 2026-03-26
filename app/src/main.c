#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "SignalMgr.h"
#include "PhaseShiftMgr.h"
#include <libopencm3/stm32/spi.h>

#define SIGNOFLIFEPORT GPIOA
#define SIGNOFLIFEPIN GPIO9   // PA9

//for our stm32 family
//alternate mapping pins supported for each of the lines we're mapping from the MCU to the Phaseshifter:
// PA5 = SCK
// PA6 = MISO
// PA7 = MOSI

//part 1
//all SPI1 on same port
#define SPI1_PORT      GPIOA
#define SPI1_CLK_PIN   GPIO5 //clk line
#define SPI1_MISO_PIN  GPIO6 //incoming response
#define SPI1_MOSI_PIN  GPIO7 //phase shift request cmd
#define SPI1_CS_PIN    SPI1_CR1 //nss pin'//NSS output enable (SOEbit in register SPI1_CR1, set SSM-0, SSOE=1)
//this configuration is used when MCU is master, NSS signal driven low when SPI enabled in master mode (SPE=1) and kept low until SPI disabled
..
void SPI1TransmitSetup(void) {

    //still haven't decided which source to clock SP1 from. it can use
    // a. PCLK2
    // b. SysCLK
    // c. HSI16
    // d. MSIK

    //enable the SPI
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);

    //now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_CLK_PIN | SPI1_MOSI_PIN | SPI1_MISO_PIN);
    gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS_PIN);

    gpio_set_af(SPI1_PORT, GPIO_AF0, SPI1_CLK_PIN | SPI1_MOSI_PIN | SPI1_MISO_PIN);

    gpio_set(SPI1_PORT, SPI1_CS_PIN);

    // now set and enable SPI1 to use master mode
    spi_reset(SPI1);
    spi_set_master_mode(SPI1);
    spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_16);
    spi_set_clock_polarity_0(SPI1);
    spi_set_clock_phase_0(SPI1);
    spi_send_lsb_first(SPI1);

    //finally enable
    spi_enable(SPI1);
}

int main(void)
{
    SPI1TransmitSetup();

    while (1)
    {

        //intended flow 

        setupSpi1(); 
        PhaseShiftRequest_t req = InitNewPhaseShiftRequest(err);
        double testShift = 205.3;
        uint8_t optBit = 0;  
        uint8_t unitAdrr = 3;

        if (req.rc =! ok)
        ResetPhaseShiftRequest(req);
        BuildPhaseShiftCmd(degreesToShift, optBit, unitAdress, &req);
        
        printf(req->packedPhaseShiftCmdString) ; 
        frame = req->PackedPhaseShiftCmd
        
        //caculate chksm to check later 
        //not yet implemented

        //now get ready to send
        
        gpio_clear(SPI1_PORT, SPI1_CS_PIN);
        gpio_set(chipselectpinwewant); 
        setupSpi1(); 
        spi_send(SPI1, frame);
        req-> PhaseShifterResponse = [] empty array 

        //response reading loop, back into our structure. 
        while (bytesToreceive) { 
        phaseshifterResponse[bytesToRecieveIndex) = spi_read(SPI1);
        index --
        }
        printf(req->phaseShifterResponse)
        //compare response to what is expected 
        Spi1Send(req-> packedPhaseShftCmdString, Spi1CRCTX 1) // to have last tx be CRC

        //depending on if we want to transmit on rising or falling edge of signal, change
        // the transition paraSmeter, need to review the data sheet
    }

    return 0;
}