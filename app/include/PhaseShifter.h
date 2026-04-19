#ifndef PHASESHIFTER_H
#define PHASESHIFTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/timer.h>

#define PE48820B_NUMSTATES 256u
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG)

#define SPI2_PS_LE_PORT GPIOB//treat the LE signal as the chip select 
#define SPI2_PS_LE_PIN GPIO12 

#define SPI2_PS_SP_PORT GPIOC
#define SPI2_PS_SP_PIN GPIO10

#define SPI2_PS_MISO_PORT GPIOC
#define SPI2_PS_MISO_PIN GPIO11//(SD01, d6 on PS side)
#define SPI2_PS_CLK_PORT GPIOB
#define SPI2_PS_CLK_PIN GPIO13

#define SPI2_PS_MOSI_PORT GPIOB
#define SPI2_PS_MOSI_PIN GPIO15

uint16_t reverseBits(uint16_t word, uint8_t numBits);
uint16_t MakePSCommand(double requestedShift_deg, bool optBit, uint8_t unitAddressWord);
void pe448spisetup(void);

#endif
