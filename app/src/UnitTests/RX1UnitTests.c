/* 
This file contains the following unit tests: 
1. Pin Toggle / Sign of Life Unit Test 
2. Phase Shifter SPI command transmission unit test. 
3. VGA SPI command transmission unit Test. 
4. VGA + Phase Shifter SPI command transmission at 'same time' (within clock cycle)
*/


//Unit test 1: Pin Toggle / Sign of Life Unit Test 
//Image 
/* 
int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(gpioPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpioPin); // required

    while (1)
    {
        SignOfLifeSignalAtMaximumSpeed(SIGNOFLIFEPORT, SIGNOFLIFEPIN); //toggle at max speed by setting and resetting BSRR for the pin 
    }

    return 0;
}
*/

//Unit test 2: Phase Shifter SPI command transmission unit test. 
//Images: 
// - RX1_RecreatePE448SpiTiming.png & RX1_RecreatePE448SpiTiming(2).png ("dont care" after LE set)
// - RX1_PE448SpiCmd.png (proof of correct cmd format) 
/* 
int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    pe448spisetup();                                   
    spi_enable(SPI2);

    //Other commands to try 
    //  uint16_t command2 = MakePSCommand(73.1, 0, 0b0011); //73.1° = 45° + 22.5° + 5.6°
    //  uint16_t command3 = MakePSCommand(128.7, 0, 0b0011); //128.7° = 90° + 22.5° + 11.2° + 5.6°
    //  uint16_t command4 = MakePSCommand(256.8, 0, 0b0011); //256.8° = 180° + 45° + 22.5° + 5.6° + 2.8° + 1.4°
    //  uint16_t command5 = MakePSCommand(33.7, 0, 0b0011); ////33.7° = 22.5° + 11.2°
    // uint16_t SingleBitCommand= 0b0010000000 for seeing unit impulse like signal propogate

    # create command 
    double requestedShift_deg = 205.3;
    bool optBit = 0;
    uint8_t unitAddressWord = 0b0011;
    uint16_t command = MakePSCommand(requestedShift_deg, optBit, unitAddressWord);

    while (1)
    {
        gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); //set the cs low
        spi_send(SPI2, command);
        phaseShifterResponse =spi_read(SPI2);
        gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN); 
        //then send another command.
        spi_send(SPI2, 0b0011111100000); //LE indifference: "dont care" about this second command. whatever we send after LE goes high should be ignored. Tested by sending a command after LE goes high and ensuring that the response is not affected
        break;
    }
    return 0;
}

*/


//Unit test 3: VGA SPI command transmission unit Test. 
/*
int main(void) 
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    f0480spisetup();                                   
    spi_enable(SPI1);
    volatile SupportedAttenuationCommand_t command = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000

    //other commands to try 
    // volatile SupportedAttenuationCommand_t command2 = ATTEN_16DB;
    // volatile SupportedAttenuationCommand_t command3 = ATTEN_8DB;
    // volatile SupportedAttenuationCommand_t command4 = ATTEN_4DB;
    // volatile SupportedAttenuationCommand_t command5 = ATTEN_2DB;

    volatile uint16_t response = 0;

    while (1)
    {
    gpio_clear(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    spi_send(SPI1, command);
    response =spi_read(SPI1);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    break;
    }
    return 0;
}


//Unit test 4: sending a command to the phase shifter and the VGA at the same time. 
/* 
int main(void) 
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz(); 
    f0480spisetup();
    pe448spisetup();
    spi_enable(SPI2);                                  
    spi_enable(SPI1);

     SupportedAttenuationCommand_t commandVGA = ATTEN_23DB; //try the max attenuation. //ATTEN_23DB = 0b01100000
    uint16_t commandPS = MakePSCommand(205.3, 0, 0b0011); //requested shift of 205.3 degrees, opt bit 0, unit address 0b0011

    //other commands to try 
    // SupportedAttenuationCommand_t command2 = ATTEN_16DB;
    // SupportedAttenuationCommand_t command3 = ATTEN_8DB;
    // SupportedAttenuationCommand_t command4 = ATTEN_4DB;
    // SupportedAttenuationCommand_t command5 = ATTEN_2DB;

  
    gpio_clear(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    gpio_clear(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);

    while (1)
    {
    spi_send(SPI2, commandPS);
    spi_send(SPI1, commandVGA);
    gpio_set(SPI1_VGA_CSB_PORT, SPI1_VGA_CSB_PIN);
    gpio_set(SPI2_PS_LE_PORT, SPI2_PS_LE_PIN);
    break;
    }
    return 0;
}

*/

