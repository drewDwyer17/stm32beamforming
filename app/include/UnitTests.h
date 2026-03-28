#include <libopencm3/stm32/gpio.h>

// //Sign of life test
// void SignOfLifeSignalAtMaximumSpeed(uint32_t gpioPort, uint32_t gpioPin) {
//         GPIO_BSRR(gpioPort) = gpioPin;  
//         GPIO_BSRR(gpioPort)  = gpioPin << 16; // reset the pin by writing to the upper half of the BSRR register
// }

