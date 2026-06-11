#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <include/PhaseShifter.h>
#include <include/Vga.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>
#include <include/HostUart.h>
#include <include/ProcessHostCmds.h>

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup(); //phase shifter spi setup  
    f0480spisetup();//vga spi setup
                                    
    spi_enable(SPI2);
    init_uart_rx();


    // //create command 
    // uint16_t command = MakePSCommand(205.3, 0, 0b0011); 
   // vga_attenuation_t attenuationCmd =  //requested attenuation of 23 dB, opt bit 0, unit address 0b0011
// spisend(SPI2, command); //send the command to the phase shifter
    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
    while (1)
    {
        if (uart_rx_data_pending) {
            uart_rx_data_pending = false;
            char extractedCmdLine[DMA_MAX_RX_CMD_LENGTH];
            processedHostCmd_t processedCmd;
            process_host_cmd(uart_rx_process_buf, extractedCmdLine, &processedCmd);
        }
    }
    return 0;
}
