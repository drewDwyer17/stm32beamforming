#define SIGNOFLIFEPORT GPIOA
#define SIGNOFLIFEPIN GPIO9   // PA9



// Function for geenrating a signal by toggling a GPIO pin at maximum speed, by setting 
//and resetting the BSRR register for the pin, and probing the pin with an oscilloscope
void SignOfLifeSignalAtMaximumSpeed(uint32_t gpioPort, uint32_t gpioPin);

//note : need to set the gpio as an output before this function can be used. 
void SignOfLifeSignalAtMaximumSpeed(uint32_t gpioPort, uint32_t gpioPin) {
        GPIO_BSRR(gpioPort) = gpioPin;  
        GPIO_BSRR(gpioPort)  = gpioPin << 16; // reset the pin by writing to the upper half of the BSRR register
}
