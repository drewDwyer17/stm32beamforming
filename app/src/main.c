#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <SignalMgr.h>
#include <PhaseShiftMgr.h>
#include <libopencm3/stm32/spi.h>>



#define SIGNOFLIFEPORT GPIOA
#define SIGNOFLIFEPIN GPIO9   // PA9

#define SPI1_CLK RCC_GPIOA
#define SPI1_MOSI RCC_GPIOB //? how do i do this
#define SPI1_MISO RCC_GPIOC
#define SPI1_CS //chip select



int main(void) 
{
    while (1)
    {
        rcc_periph_clock_enable(RCC_SPI1);
        rcc_periph_clock_enable(SPI1_CLK);    //alternate function pin for SPI CLK
        rcc_periph_clock_enable(SPI1_MOSI );//alternate function pin for SPI SI
        rcc_periph_clock_enable(SPI1_MISO); //alternate function pin for Spi RX
  
        
        gpio_set_mode(GPIOB, GPIO_MODE_AF, PUPD);//alterante,GPIO2|GPIO3|GPIO4); //3 alternate functions 
        //
        spi_set_master_mode(SPI1);
        spi_set_clock_polarity_0(SPI1);
        spi_set_clock_phase_0(SPI1);
        spi_set_standard_mode(SPI1);

        uint_32_t baudRatePrescaler = ispi_set_baudrate_prescaler(SPI1, 10000); // what baud rate? 

        spi_init_master(SPI1, baudratePrescaler, SPI_CR1_CPOL, SPI_CR1_CPHA, 0); 
        //not quite sure
        
        spi_enable(); 

        // now send? 
        double degreesToShift = 45.0;
        bool optBit = 0; 
        uint8_t unitAdress = 3; 
        PhaseShiftMgr_Error_t rc; 

        PhaseShiftRequest_t req = InitNewPhaseShiftRequest(err); 
        
        if (req.rc =! ok) 
        ResetPhaseShiftRequest(req); 
        PackedPhaseShiftCmd_t PhaseShiftCommand = BuildPhaseShiftCmd(degreesToShift, optBit, unitAdress, &req); //return the rc of this function to the req structure that we intiailize, but the
        //en return a 13 bit command as a packed tupe. he packing function should be wihtin this api to make it readable 
        
        if (req.err = Et)
        //perform the CRC check on the command. Function for this isnt implemented 

        //now we're ready to send it to the phaes shifter. 
        send it byte by byte 

        spi_send_phase_shift(phaseshiftcmd
        BuildPhaseShiftCmd(degreesToShift,   
        spi_send

        //double check what is expected 


        
        
        // //need to select for RCC_APB1ENR or RCC_APB2ENR (see deepwiki: lnk)
        //apb1 is slowspeed peripheral bus, apb2 is highspeed. this is what the f0 allows for 
        //for the spi communication we need alternate function gpio types 
        gpio_set_af(SPI1, ); 
        // spi_enable_crc(SPI1)
        // spi_set_full_duplex_mode(SPI1)


        
        //ok these are the things we need. The stm32 is the controller here 
        // configure MOSI : output pin, type general output (data)
        // MISO input data pin : input pin, no pdpu
        // CLK this will be used to synchronize the spi communication 



//only enable spi at the end when everything is configured 
        spi_enable()

        // rcc_clock_setup_in_hse_8mhz_out_48mhz();
        // rcc_periph_clock_enable(RCC_GPIOA);
        // SignOfLifeSignalAtMaximumSpeed(SIGNOFLIFEPORT, SIGNOFLIFEPIN);

        
    }
    return 0;
}


