#ifndef HOSTUART_H
#define HOSTUART_H

#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#define UART_PORT USART1
#define UART_BAUDRATE 115200

#define RS485_RX_EN_PORT GPIOC
#define RS485_RX_EN_PIN GPIO12

#define RS485_DRIVER_EN_PORT GPIOC
#define RS485_DRIVER_EN_PIN GPIO11

#define RS485_TX_PORT GPIOB
#define RS485_TX_PIN GPIO6

#define RS485_RX_PORT GPIOB
#define RS485_RX_PIN GPIO7

#define DMA_MAX_RX_CMD_LENGTH 64

#define UART_RX_DMA DMA1_BASE
#define UART_RX_DMA_RCC RCC_DMA1
#define UART_RX_DMA_CHANNEL DMA_CHANNEL3
#define UART_RX_DMA_IRQ NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ

/*
Functions for handling UART commands from another device. 
1. command received as string of bytes at USART1 Rx pin, unknown length but expected to be <64 bytes. 
2. DMA channel is set up to transfer the incoming bytes to a buffer in memory (uart_rx_dma_buf) as it arrives. 
3. After atleast one byte is received, When the device stops transmitting, USART IDLE interrupt fires


4. In the ISR for the USART idle interrupt
- freeze the DMA channel
- compute the num bytes we've received, 
- pushuart_rx_dma_buf to a processing buffer to free dma buffer for next cmd 
- then we restart the DMA channel 
- set a flag (uart_rx_data_pending) to get CPU attenuation to process the command. 

5. CPU monitors the flag in main loop. When set, CPU starts parsing pong, extracts a cmd and executes (See processHostCmds.c)
`
*/

extern uint8_t uart_rx_process_buf[DMA_MAX_RX_CMD_LENGTH];
extern volatile bool uart_rx_data_pending;

void init_uart_rx(void);
void deinit_uart_rx(void);

#endif