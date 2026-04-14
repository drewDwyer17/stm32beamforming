#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "PhaseShifter.h"
#include "Vga.h"
#include <stdint.h>
#include <libopencm3/stm32/timer.h>


int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 

    f0480spisetup();                                   

    spi_enable(SPI1);

    volatile SupportedAttenuationCommand_t  command = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000
    volatile uint16_t response = 0;

    while (1)
    {
    gpio_clear(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    spi_send(SPI1, command);
    response =spi_read(SPI1);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    break;
    }
    return 0;
}










*/
