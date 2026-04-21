#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <include/PhaseShifter.h>
#include <include/Vga.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>


int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup();                                   
    spi_enable(SPI2);

    //optional volatile uint16_t SingleBitCommand= 0b0010000000
    volatile uint16_t phaseShifterResponse = 0;

    //create command 
    uint16_t command = MakePSCommand(205.3, 0, 0b0011); //requested shift of 205.3 degrees, opt bit 0, unit address 0b0011

    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
        //other valid phase shifts to test: 
    
    //  uint16_t command2 = MakePSCommand(73.1, 0, 0b0011); //73.1° = 45° + 22.5° + 5.6°
    //  uint16_t command3 = MakePSCommand(128.7, 0, 0b0011); //128.7° = 90° + 22.5° + 11.2° + 5.6°
    //  uint16_t command4 = MakePSCommand(256.8, 0, 0b0011); //256.8° = 180° + 45° + 22.5° + 5.6° + 2.8° + 1.4°
    //  uint16_t command5 = MakePSCommand(33.7, 0, 0b0011); ////33.7° = 22.5° + 11.2°

    while (1)
    { //set the cs low
        spi_send(SPI2, command);
        phaseShifterResponse =spi_read(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        break;
    }
    return 0;
}

// int main(void) 
// {
//     rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
//     f0480spisetup();                                   
//     spi_enable(SPI1);
//     volatile SupportedAttenuationCommand_t command = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000

//     //other commands to try 
//     // volatile SupportedAttenuationCommand_t command2 = ATTEN_16DB;
//     // volatile SupportedAttenuationCommand_t command3 = ATTEN_8DB;
//     // volatile SupportedAttenuationCommand_t command4 = ATTEN_4DB;
//     // volatile SupportedAttenuationCommand_t command5 = ATTEN_2DB;

//     volatile uint16_t response = 0;

//     while (1)
//     {
//     gpio_clear(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
//     spi_send(SPI1, command);
//     response =spi_read(SPI1);
//     gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
//     break;
//     }
//     return 0;
// }



//Unit test: sending a command to the phase shifter and the VGA at the same time. 

int main(void) 
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    f0480spisetup();
    pe448spisetup();
    spi_enable(SPI2);                                  
    spi_enable(SPI1);
    volatile SupportedAttenuationCommand_t commandVGA = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000
    uint16_t commandPS = MakePSCommand(205.3, 0, 0b0011); //requested shift of 205.3 degrees, opt bit 0, unit address 0b0011

    //other commands to try 
    // volatile SupportedAttenuationCommand_t command2 = ATTEN_16DB;
    // volatile SupportedAttenuationCommand_t command3 = ATTEN_8DB;
    // volatile SupportedAttenuationCommand_t command4 = ATTEN_4DB;
    // volatile SupportedAttenuationCommand_t command5 = ATTEN_2DB;

    volatile uint16_t response = 0;
    
    gpio_clear(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);

    while (1)
    {
    response =spi_read(SPI1);
    spi_send(SPI2, commandPS);
    spi_send(SPI1, commandVGA);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
    break;
    }
    return 0;
}

