#ifndef VGA_H
#define VGA_H

#include <libopencm3/stm32/gpio.h>

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

#define SPI1_VGA_CSB_PORT  GPIOA
#define SPI1_VGA_CSB_PIN   GPIO4

#define SPI1_VGA_MISO_PORT GPIOA
#define SPI1_VGA_MISO_PIN  GPIO6

#define SPI1_VGA_CLK_PORT  GPIOA
#define SPI1_VGA_CLK_PIN   GPIO5

#define SPI1_VGA_MOSI_PORT GPIOA
#define SPI1_VGA_MOSI_PIN  GPIO7

void f0480spisetup(void);

#endif
