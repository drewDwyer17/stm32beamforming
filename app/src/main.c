#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

/* Use the first pin for sign of life */
#define OUTPUT_GPIO_SQUAREWAVE_PORT GPIOA
#define OUTPUT_GPIO_SQUAREWAVE_PIN  GPIO9   /* PA1 */

static void clock_setup(void) {
    /* Configure system clock to 48 MHz using 8 MHz external crystal.
       Trying to use the fastest clock source for sign of life. */
    rcc_clock_setup_in_hse_8mhz_out_48mhz();

    /* Enable clock for the port used by the square wave output pin */
    rcc_periph_clock_enable(RCC_GPIOA);
}

int main(void) {

    clock_setup();

    /* Configure the square wave pin as push-pull output, no pull-up/down */
    gpio_mode_setup(OUTPUT_GPIO_SQUAREWAVE_PORT,
                    GPIO_MODE_OUTPUT,
                    GPIO_PUPD_NONE,
                    OUTPUT_GPIO_SQUAREWAVE_PIN);

    while (1) {
        /* Toggle the pin continuously to produce a square wave */
        gpio_toggle(OUTPUT_GPIO_SQUAREWAVE_PORT,
                    OUTPUT_GPIO_SQUAREWAVE_PIN);
    }

    return 0;
}