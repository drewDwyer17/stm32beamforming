#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <include/PhaseShifter.h>
#include <include/Vga.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>
#include <include/HostUart.h>
#include <include/ProcessHostCmds.h>

// int main(void)
// {
//     rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
//     pe448spisetup(); //phase shifter spi setup  
//     // f0480spisetup();//vga spi setup
                                    
//     spi_enable(SPI2);
//     // spi_enable(SPI1);
//     // init_uart_rx();


//     // //create command 
//     // uint16_t command = MakePSCommand(205.3, 0, 0b0011); 
//    // vga_attenuation_t attenuationCmd =  //requested attenuation of 23 dB, opt bit 0, unit address 0b0011
// // spisend(SPI2, command); //send the command to the phase shifter
//     gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
//     while (1)
//     {
//         // if (uart_rx_data_pending) {
//         //     uart_rx_data_pending = false;
//         //     char extractedCmdLine[DMA_MAX_RX_CMD_LENGTH];
//         //     processedHostCmd_t processedCmd;
//         //     process_host_cmd(uart_rx_process_buf, extractedCmdLine, &processedCmd);
//         // }


//     }
//     return 0;
// }

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup();                                   
    spi_enable(SPI2);

    //Other commands to try 
    //  uint16_t command2 = MakePSCommand(73.1, 0, 0b0011); //73.1° = 45° + 22.5° + 5.6°
    //  uint16_t command3 = MakePSCommand(128.7, 0, 0b0011); //128.7° = 90° + 22.5° + 11.2° + 5.6°
    //  uint16_t command4 = MakePSCommand(256.8, 0, 0b0011); //256.8° = 180° + 45° + 22.5° + 5.6° + 2.8° + 1.4°
    //  uint16_t command5 = MakePSCommand(33.7, 0, 0b0011); ////33.7° = 22.5° + 11.2°
    // uint16_t SingleBitCommand= 0b0010000000 for seeing unit impulse like signal propogate

    
    float requestedShift_deg = 205.3;
    bool optBit = 0;
    uint8_t unitAddressWord = 0b0011;
    uint16_t command = MakePSCommand(requestedShift_deg, optBit, unitAddressWord);

    while (1)
    {
        gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low
        spi_send(SPI2, command);
        
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        //then send another command.
        // spi_send(SPI2, 0b0011111100000); //LE indifference: "dont care" about this second command. whatever we send after LE goes high should be ignored. Tested by sending a command after LE goes high and ensuring that the response is not affected
        break;
    }
    return 0;
}