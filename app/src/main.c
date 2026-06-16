/* 
Note for Elias about using the CAN extended 29 bit ID as a way to filter on the message id
we dont necessarily need to be able to target 28 ids
we can use some of the message id bits to compute off the bat which type of message we are using 
there is a structure that is populated with the id upon the CAN message reception ( see beamformerCan.h, its the: unprocessedCanCmd_t, there is an ID field)
we can mask the id field in the first 3 bits, expecting that those first 3 bits will tell us which type of command 
(VGA, PS or Combined) we are working with 
in ProcessingHostCmds.c, make sure that when you are processing a host can message, 
instead of using the first byte of the CAN data payload as the idenitfier for the command ID, 
switch on the first 3 bits of the command 




*/



#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <include/PhaseShifter.h>
#include <include/Vga.h>
#include <include/HostUart.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>
#include <stdbool.h>
#include <include/ProcessHostCmds.h>
#include <include/beamformerCan.h>

extern volatile bool can_data_pending; //flag for main loop
extern uint8_t can_data_rx[8]; //populated by CAN ISR when can message passes acceptance filters set for this mcu 


int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz(); 
    pe448spisetup(); //phase shifter spi setup connected to this device
    f0480spisetup();//vga spi setup
                                    
    spi_enable(SPI2);
    spi_enable(SPI1);
    init_uart_rx();

    canSetUpClocks(); 
    canSetUpAcceptanceFilters(CAN_FILTER_MCU1_MASK); //if this is self 

//Examples
    //  uint16_t command2 = MakePSCommand(73.1, 0, 0b0011); //73.1° = 45° + 22.5° + 5.6°
    //  uint16_t command3 = MakePSCommand(128.7, 0, 0b0011); //128.7° = 90° + 22.5° + 11.2° + 5.6°
    //  uint16_t command4 = MakePSCommand(256.8, 0, 0b0011); //256.8° = 180° + 45° + 22.5° + 5.6° + 2.8° + 1.4°
    //  uint16_t command5 = MakePSCommand(33.7, 0, 0b0011); ////33.7° = 22.5° + 11.2°
    // uint16_t SingleBitCommand= 0b0010000000 
   // vga_attenuation_t attenuationCmd =  //requested attenuation of 23 dB, opt bit 0, unit address 0b0011
   // spisend(SPI2, command); //send command to phase shifter
   //spisend(SPI1, attenuationCmd); //vga


    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);

    while (1)
    {
        // Can message received and passed acceptance filters
        if (can_data_pending) {
            can_data_pending = false; //clear
            process_host_cmd_can(can_data_rx);
        }
        else if (uart_rx_data_pending) {
            uart_rx_data_pending = false;
            char extractedCmdLine[DMA_MAX_RX_CMD_LENGTH]; //get new structures
            processedHostCmd_t processedCmd;
            process_host_cmd_uart(uart_rx_process_buf, extractedCmdLine, &processedCmd);
        }

    }
    return 0;
}