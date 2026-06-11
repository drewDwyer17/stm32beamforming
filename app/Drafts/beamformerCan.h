//SN65HVD230 is the CAN transciever we are connecting to the cpu in RX4 configuration
//https://www.ti.com/lit/ds/symlink/sn65hvd230.pdf?ts=1778578757681&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FSN65HVD230

#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


void setUpCan1(void) 
{ 
    rcc_periph_clock_enable(RCC_CAN1);
    rcc_periph_clock_enable(RCC_GPIOA);

    // CAN1_TX = PA12
    // CAN1_RX = PA11

    //the CAN peripheral is typically connected to GPIO pins through the MCU’s alternate function (AF) system.
    //canTX and canRX pins must be mapped to alternate functions for CAN bus on our MCU 
    //
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF9, GPIO11 | GPIO12);

    can_reset(CAN1);
    can_init(CAN1, false, false, false, false, false, false, 1, 13, 2, 4, false, false);

}