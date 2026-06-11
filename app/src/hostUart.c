/*
Process for receiving host command sent over UART from terminal on PC. 
1. host command received as string of bytes at USART1 Rx pin, unknown length but expected to be <64 bytes. 
2. DMA channel is set up to transfer the incoming bytes to a buffer in memory (uart_rx_dma_buf) as it arrives. 
3. USART IDLE interrupt fires, indicating that at least one byte has been received 
and no more have arrived within the idle timeout period. This indicates that the host is done 
sending data and we can process it. 
4. In the ISR for the USART idle interrupt, we:
- freeze the DMA channel
- compute the num bytes we've received, 
- flush (memcpy) recieved byte count from uart_rx_dma_buf (ping) buf to a processing buffer (pong) to free ping for next cmc 
- then we restart the DMA channel 
- set a flag (uart_rx_data_pending) to get CPU attenuation to process the command. 

5. CPU monitors the flag in main loop. When set, CPU starts parsing pong, extracts a cmd and executes (See processHostCmds.c)

*/

#include <include/HostUart.h>
#include <include/ProcessHostCmds.h>
#include <libopencm3/stm32/dma.h>
#include <string.h>

static uint8_t uart_rx_dma_buf[DMA_MAX_RX_CMD_LENGTH];
uint8_t uart_rx_process_buf[DMA_MAX_RX_CMD_LENGTH];
volatile bool uart_rx_data_pending = false;

/*
 * DMA RX is split into two functions:
 * 1. init_dma_rx() -- called once, sets up DMA channel params 
 *  2. restart_dma_rx(), called to refresh the DMA dest memory address and transfer count, and then re-enable the DMA channel to start the next transfer
 * The DMA buffer then is free to be refilled with incoming UART rx bytes carrying next host comm.  
 */
static void uart_dma_rx_init(void)
{
    dma_channel_reset(UART_RX_DMA, UART_RX_DMA_CHANNEL);

    // 1. set DMA_CPARx - peripheral data address to dma transfer from
    dma_set_peripheral_address(UART_RX_DMA, UART_RX_DMA_CHANNEL, (uint32_t)&USART_RDR(UART_PORT));

    // 2. set DMA_CMARx to DMA transfer to mem
    dma_set_memory_address(UART_RX_DMA, UART_RX_DMA_CHANNEL, (uint32_t)uart_rx_dma_buf);

    // 3. set DMA_CNDTRx specify how much 
    dma_set_number_of_data(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_MAX_RX_CMD_LENGTH);

    // 4. parameter configurations in DMA_CCRx
    // a. priority
    dma_set_priority(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_CCR_PL_HIGH);

    // b.data transfer direction - from peripheral to memory
    dma_set_read_from_peripheral(UART_RX_DMA, UART_RX_DMA_CHANNEL);

    // c.memory incremented mode (byte by byte)
    dma_enable_memory_increment_mode(UART_RX_DMA, UART_RX_DMA_CHANNEL);

    // d.peripheral and memory data size - 8 bit for USART data register
    dma_set_peripheral_size(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_CCR_MSIZE_8BIT);

    /*
     * Transfer-complete interrupt is a safety interrupt.
     * Normal host command completion signal is expected to come from the USART IDLE interrupt.
     * TC means the DMA buffer filled before IDLE occurred. (Shouldnt usually happen because we made the DMA buffer big enough to receive full comm)
     */
    dma_enable_transfer_complete_interrupt(UART_RX_DMA, UART_RX_DMA_CHANNEL);

    // set EN in DMA_CCRx to start the transfer
    dma_enable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL);
}

/*
* The channel is already configured by init_dma_rx()
* when we clear the uart idle interrupt, need to 
- restart the dma channel, 
- reload transfer counter register CRndr
- renable the channel 
*/
static void restart_dma_rx(void)
{
    dma_disable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL);
    dma_set_memory_address(UART_RX_DMA,UART_RX_DMA_CHANNEL, (uint32_t)uart_rx_dma_buf);
    dma_set_number_of_data(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_MAX_RX_CMD_LENGTH);
    dma_enable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL);
}

void init_uart_rx(void)
{
     //Uart config for RS485-UART reception
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOB);

    
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE, RS485_RX_EN_PIN | RS485_DRIVER_EN_PIN);

    gpio_clear(GPIOC, RS485_DRIVER_EN_PIN);
    gpio_clear(GPIOC, RS485_RX_EN_PIN);

    //set the uart as alternate function mode and assign right pins using the table in TRM
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, RS485_TX_PIN | RS485_RX_PIN);
    gpio_set_af(GPIOB, GPIO_AF0, RS485_TX_PIN | RS485_RX_PIN);

    //configure uart comm config (baudrate, data bits, stop bits, parity, flow control)
    usart_set_baudrate(UART_PORT, UART_BAUDRATE);
    usart_set_databits(UART_PORT, 8);
    usart_set_stopbits(UART_PORT, USART_STOPBITS_1);
    usart_set_parity(UART_PORT, USART_PARITY_NONE);
    usart_set_mode(UART_PORT, USART_MODE_TX_RX);
    usart_set_flow_control(UART_PORT, USART_FLOWCONTROL_NONE);

    nvic_set_priority(NVIC_USART1_IRQ, 0); // Uart IDLE: higher priority
    nvic_set_priority(UART_RX_DMA_IRQ, 1); //DMA Transfer complete interrupt as a backup. Will execute same logic in ISR as Idle to trigger processing. 

    //config and enable DMA for UART rx. Starts transfer of fifo to DMArx buffer. 
    uart_dma_rx_init();

    // Allows USART RX events to generate DMA requests.
    usart_enable_rx_dma(UART_PORT); 

    usart_enable(UART_PORT); //finally, enable the USART peripheral to receive data

    /*
     * IDLE interrupt fires when RX line goes idle after at least one byte. We're not sure how long the command will be, so this is a better signal for interrupt. 
     * we'll still use DMA complete as a backup interrupt
     */
    usart_enable_idle_interrupt(UART_PORT);
    nvic_enable_irq(UART_RX_DMA_IRQ);
    nvic_enable_irq(NVIC_USART1_IRQ);
}

/*
 * - UART IDLE interrupt -- this is first priority and the normal path
 */
void usart1_isr(void)
{
    if (usart_get_flag(UART_PORT, USART_ISR_IDLE)) {
        USART_ICR(UART_PORT) = USART_ICR_IDLECF; //clear idle line IR flag

        dma_disable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL); //disable DMA temporarily so we can read the transfer count

        uint16_t recieved = DMA_MAX_RX_CMD_LENGTH - dma_get_number_of_data(UART_RX_DMA, UART_RX_DMA_CHANNEL); //calculate num bytes that have been transfered from USART fifo to DMA buff

        memcpy(uart_rx_process_buf, uart_rx_dma_buf, recieved); //flush the received bytes in the DMA buffer to the process buffer so DMA buffer can refill with next cmd.
        restart_dma_rx();
        uart_rx_data_pending = true;
    }
}

// transfer complete (backup) interrupt, which triggersif the command is too long for the buffer. We aren't expecting this.  
void dma1_channel2_3_dma2_channel1_2_isr(void)
{
    if (dma_get_interrupt_flag(UART_RX_DMA,UART_RX_DMA_CHANNEL, DMA_TCIF)) {

        dma_clear_interrupt_flags(UART_RX_DMA, UART_RX_DMA_CHANNEL, DMA_TCIF);

        /*
         * Buffer filled before IDLE occurred.
         * This may mean:
         * - command is exactly DMA_MAX_RX_CMD_LENGTH bytes,
         * - command is too long,
         * - or multiple commands arrived with no idle gap.
         */
        dma_disable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL); //disable DMA temporarily so we can read the transfer count

        uint16_t recieved = DMA_MAX_RX_CMD_LENGTH - dma_get_number_of_data(UART_RX_DMA, UART_RX_DMA_CHANNEL); //calculate num bytes that have been transfered from USART fifo to DMA buff

        memcpy(uart_rx_process_buf, uart_rx_dma_buf, recieved); //flush the received bytes in the DMA buffer to the process buffer so DMA buffer can refill with next cmd.
        restart_dma_rx();
        uart_rx_data_pending = true;

    }
}

//unused for now but may be useful for future
void deinit_uart_rx(void)
{
    usart_disable(UART_PORT);
    dma_disable_channel(UART_RX_DMA, UART_RX_DMA_CHANNEL);
}