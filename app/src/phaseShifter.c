#include <include/PhaseShifter.h>

// The phase word and address field need bit order reversed before packing, see datasheet timing diagram and command structure.
uint16_t reverseBits(uint16_t word, uint8_t numBits)
{
    uint16_t reversed = 0;

    for (uint8_t i = 0; i < numBits; i++) {
        reversed = (uint16_t)((reversed << 1) | (word & 0x1u));
        word >>= 1;
    }

    return reversed;
}

uint16_t MakePSCommand(double requestedShift_deg, bool optBit, uint8_t unitAddressWord)
{
    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);
    phaseSetWord = reverseBits(phaseSetWord, 8); // reverse the 8-bit phase field
    unitAddressWord = reverseBits(unitAddressWord, 4);
    uint16_t command =((phaseSetWord & 0xFFu) << 5) | ((optBit & 0x1u) << 4) |(unitAddressWord & 0x0Fu);

    return command;
} 

void pe448spisetup(void)
{
    rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_GPIOA); 
    rcc_periph_clock_enable(RCC_GPIOB); //for CLK, MOSI, and LE
    rcc_periph_clock_enable(RCC_GPIOC); //for MISO and SP (serial progrmaming select pin)

    gpio_mode_setup(SPI2_PS_SP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_PS_SP_PIN); 
    gpio_set(SPI2_PS_SP_PORT, SPI2_PS_SP_PIN); //PS SP pin HIGH for serial communication  

    //now set the pin modes as outputs and AF 
    gpio_mode_setup(SPI2_PS_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_CLK_PIN); 
    gpio_mode_setup(SPI2_PS_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_MISO_PIN); 
    gpio_mode_setup(SPI2_PS_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_PS_MOSI_PIN); 

    //set latch enable (CS) as general output, Alternate functions for SPI bus lines
    gpio_mode_setup(SPI2_PS_LE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_PS_LE_PIN);  
    gpio_set_af(SPI2_PS_CLK_PORT, GPIO_AF0, SPI2_PS_CLK_PIN); 
    gpio_set_af(SPI2_PS_MOSI_PORT, GPIO_AF0, SPI2_PS_MOSI_PIN); 
    gpio_set_af(SPI2_PS_MISO_PORT, GPIO_AF0, SPI2_PS_MISO_PIN); 

    spi_set_data_size(SPI2, SPI_CR2_DS_13BIT); //our command is 13 bits long. 
    spi_fifo_reception_threshold_16bit(SPI2);
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 0, SPI_CR1_MSBFIRST); //send MSB first. We've already flipped the command elements to LSB first as required during command construction. 
    
}

/*PS Unit Tests
Example base usage (Reproduce datasheet PS & Spi timing diagram) 

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup();                                   
    spi_enable(SPI2);

    #optional volatile uint16_t SingleBitCommand= 0b0010000000 for seeing unit impulse like signal propogate
    volatile uint16_t phaseShifterResponse = 0;

    # create command 
    double requestedShift_deg = 205.3;
    bool optBit = 0;
    uint8_t unitAddressWord = 0b0011; //address of the unit we're trying to control, 4 bits for up to 16 units.
    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);
    phaseSetWord = reverseBits(phaseSetWord); //reverse the bits of the phase set word to be LSB first
    unitAddressWord = reverseBits(unitAddressWord);
    command = (phaseSetWord << 5) | (optBit << 4) | unitAddressWord;

    while (1)
    {
        gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low
        spi_send(SPI2, command);
        phaseShifterResponse =spi_read(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        //then send another command.
        spi_send(SPI2, 0b0011111100000); //this shouldn't matter. Based on the datasheet, whatever we send after LE goes high should be ignored. Tested by sending a command after LE goes high and ensuring that the response is not affected
        break;
    }
    return 0;
}

*/






























