#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "src/phaseShifter.c"
#include <include/Vga.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>



int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 

    pe448spisetup();                                   

    spi_enable(SPI2);
    
    double requestedShift_deg = 205.3;
    bool optBit = 0;
    uint8_t unitAddressWord = 0b0011; //address of the unit we're trying to control, 4 bits for up to 16 units.

    volatile uint16_t command = 0b0010000000000; 
    volatile uint16_t phaseShifterResponse = 0;

    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);

    phaseSetWord = reverseBits(phaseSetWord); //reverse the bits of the phase set word to be LSB first
    unitAddressWord = reverseBits(unitAddressWord);

    
    command = (phaseSetWord << 5) | (optBit << 4) | unitAddressWord;

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
        //  a different command after the first one and seeing if the response changes.
        //  #oscilloscope readings confirm
         
        break;

    }
    return 0;
}
