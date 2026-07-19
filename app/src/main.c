#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <include/PhaseShifter.h>
#include <include/Vga.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>
#include <stdbool.h>

void hard_fault_handler(void) { 

    // Hard Fault Status Register
    volatile uint32_t hfsr = *(uint32_t *)0xE000ED2C;
    
    // Configurable Fault Status Register
    volatile uint32_t cfsr = *(uint32_t *)0xE000ED28;
    volatile uint8_t  mmfsr = cfsr & 0xFF; //Memory Management Fault Status
    volatile uint8_t  bfsr  = (cfsr >> 8) & 0xFF; //Bus Fault status 
    volatile uint16_t ufsr  = (cfsr >> 16); //Usage Fault Status Reg 
    
    // Fault Address Registers
    volatile uint32_t mmar = *(uint32_t *)0xE000ED34; //MemManage Fault Address
    volatile uint32_t bfar = *(uint32_t *)0xE000ED38; //Bus Fault Address
    
    // Debug Fault Status Register
    volatile uint32_t dfsr = *(uint32_t *)0xE000ED30;
    
    __asm__("bkpt #0");

    while(1);
}
// Inspect in gdb: p/x hfsr, p/x cfsr, p/x mmfsr, p/x bfsr, p/x ufsr, p/x mmar, p/x bfar, p/x dfsr



int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup();                                   
    spi_enable(SPI2);

    //create command 
    uint16_t command = MakePSCommand(205.3f, 0, (uint8_t)0b0011); //requested shift of 205.3 degrees, opt bit 0, unit address 0b0011

    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
    while (1)
    { //set the cs low
        spi_send(SPI2, command);
        // phaseShifterResponse =spi_read(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        break;
    }
    
    while(1);
    
    return 0;
}
