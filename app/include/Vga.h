#ifndef VGA_H
#define VGA_H
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <stdint.h>

#define SPI1_VGA_CSB_PORT  GPIOA
#define SPI1_VGA_CSB_PIN   GPIO4

#define SPI1_VGA_MISO_PORT GPIOA
#define SPI1_VGA_MISO_PIN  GPIO6

#define SPI1_VGA_CLK_PORT  GPIOA
#define SPI1_VGA_CLK_PIN   GPIO5

#define SPI1_VGA_MOSI_PORT GPIOA
#define SPI1_VGA_MOSI_PIN  GPIO7


// max serial clock speed for vga is 10 mhz
// serial control mode
// data clocked in LSB first 

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
    ATTEN_0DB  = 0b00000000,
    ATTEN_1DB  = 0b00000100,
    ATTEN_2DB  = 0b00001000,
    ATTEN_4DB  = 0b00010000,
    ATTEN_8DB  = 0b00100000,
    ATTEN_16DB = 0b01000000,
    ATTEN_22DB = 0b01011000,
    ATTEN_23DB = 0b01100000
} SupportedAttenuationCommand_t;


void f0480spisetup(void);

#endif
