#include <include/Vga.h>

void f0480spisetup(void)
{
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(SPI1_VGA_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_VGA_CSB_PIN);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN); // set CSB high. " When CSb is high (> VIH), the CLK input is disabled and serial 
                                                    // data (DATA) is not clocked into the shift register.  It is recommended that CSb be pulled high (>VIH) when 
                                                    // the device is not being programmed.""

    // now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI1_VGA_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_CLK_PIN);
    gpio_mode_setup(SPI1_VGA_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_MISO_PIN);
    gpio_mode_setup(SPI1_VGA_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_MOSI_PIN);

    gpio_mode_setup(SPI1_VGA_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_VGA_CSB_PIN); // set chip select as general output

    gpio_set_af(SPI1_VGA_CLK_PORT, GPIO_AF0, SPI1_VGA_CLK_PIN);
    gpio_set_af(SPI1_VGA_MOSI_PORT, GPIO_AF0, SPI1_VGA_MOSI_PIN);
    gpio_set_af(SPI1_VGA_MISO_PORT, GPIO_AF0, SPI1_VGA_MISO_PIN);

    spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
    spi_fifo_reception_threshold_8bit(SPI1);
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 0, SPI_CR1_LSBFIRST);
}


/* Unit Testing 
Example base usage  (confirmed with testing April 13)

// int main(void) 
// {
//     rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
//     f0480spisetup();                                   
//     spi_enable(SPI1);
//     volatile SupportedAttenuationCommand_t command = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000
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


*/
