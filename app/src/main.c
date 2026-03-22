#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <SignalMgr.h>

#define SIGNOFLIFEPORT GPIOA
#define SIGNOFLIFEPIN GPIO9   // PA9


int main(void) 
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    while (1)
    {
        SignOfLifeSignalAtMaximumSpeed(SIGNOFLIFEPORT, SIGNOFLIFEPIN);
    }
    return 0;
}


