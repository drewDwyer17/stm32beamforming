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
    uint16_t command = MakePSCommand(205.3, 0, 0b0011); //requested shift of 205.3 degrees, opt bit off, unit address 0b0011

    while (1)
    {
        gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low
        spi_send(SPI2, command);
        phaseShifterResponse =spi_read(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        break;
    }
    return 0;
}


