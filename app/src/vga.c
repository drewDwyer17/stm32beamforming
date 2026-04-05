#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

// max serial clock speed for vga is 10 mhz

// says that we use spi 3 bus for CSB, serial data and clock

// band select configured on hardware

// serial control mode
// data clocked in LSB first so that is consistent
// vga has a CSb pin, that is an inhibit freature, so that CLK bus noise is ignored (data
// not clocked when CSb is high. When the device is not programmed, pull Csb high, and then when programming
// pull Csb low.

// VGA_CLK

// VGA_CSB

// VGA_MISO (SPI_DATA)

// example
// for 8 db attenuation we have
// D0 = x
// D1 = x
// D2 = L
// D3 = L
// D4 = L
// D5 = H
// D6 = L
// D7 = x

// for max attenuation (23db) we have

// D0 = x
// D1 = x
// D2 = H
// D3 = H
// D4 = H
// D5 = L
// D6 = H
// D7 = x

typedef enum SupportedAttenuationCommand
{
    // we only care about d2-d6
    ATTEN_0DB  = 0b00000000, // 00000 << 2
    ATTEN_1DB  = 0b00000100, // 00001 << 2
    ATTEN_2DB  = 0b00001000, // 00010 << 2
    ATTEN_4DB  = 0b00010000, // 00100 << 2
    ATTEN_8DB  = 0b00100000, // 01000 << 2
    ATTEN_16DB = 0b01000000, // 10000 << 2
    ATTEN_22DB = 0b01011000, // 10110 << 2
    ATTEN_23DB = 0b01100000  // 11000 << 2
} SupportedAttenuationCommand_t;

// need to use alternate function pins for SPI1

// SPI1_SCK = PA5
// SPI1_MISO = PA6
// SPI1_MOSI = PA7

// CPOL 0, CPHA 0, LSB, SPI1

#define SPI1_VGA_CSB_PORT  GPIOA
#define SPI1_VGA_CSB_PIN   GPIO4

#define SPI1_VGA_MISO_PORT GPIOA
#define SPI1_VGA_MISO_PIN  GPIO6

#define SPI1_VGA_CLK_PORT  GPIOA
#define SPI1_VGA_CLK_PIN   GPIO5

#define SPI1_VGA_MOSI_PORT GPIOA
#define SPI1_VGA_MOSI_PIN  GPIO7

void f0480spisetup(void)
{
    // enable the SPI
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);

    // need to set the PS SP pin HIGH for serial communication
    gpio_mode_setup(SPI1_VGA_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_VGA_CSB_PIN);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN); // set CS high for serial communication

    // now set the pin modes as outputs and AF as specified above
    gpio_mode_setup(SPI1_VGA_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_CLK_PIN);
    gpio_mode_setup(SPI1_VGA_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_MISO_PIN);
    gpio_mode_setup(SPI1_VGA_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI1_VGA_MOSI_PIN);

    gpio_mode_setup(SPI1_VGA_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_VGA_CSB_PIN); // set chip select as general output

    // set CS high to prevent data transfer
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);

    gpio_set_af(SPI1_VGA_CLK_PORT, GPIO_AF0, SPI1_VGA_CLK_PIN);
    gpio_set_af(SPI1_VGA_MOSI_PORT, GPIO_AF0, SPI1_VGA_MOSI_PIN);
    gpio_set_af(SPI1_VGA_MISO_PORT, GPIO_AF0, SPI1_VGA_MISO_PIN);

    // SDO1 phase shifter output is D6 of the PS schematic, need to map to MISO. D6 is mapped to PC12 MCU side
    // now set and enable SPI1 to use master mode and
    // set the idle state of the clock Low for CPOL = 0 (TRM "The idle state of SCK must correspond to the polarity selected in the SPIx_CR1 register (by
    // pulling up SCK if CPOL=1 or pulling down SCK if CPOL=0)"

    // Note april 5 think i need to change the CPHA to 0.

    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 0, 0, SPI_CR1_LSBFIRST);
    // our Phase shifter expects CPHA = 1, CPOL = 0, and we want to send LSB first.
    // Baudrate is set to 3MHz (48MHz/16) to keep the signal slow so we can see it

    spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
    // our command size is 13 bits.
    // I think we should pad it to be 16 bits. The Ps will filter them out in one example.

    spi_fifo_reception_threshold_16bit(SPI1); // make sure to capture all of the receive bits
}