 PhaseShiftRequest_t req;
    req.requestedShift_deg = 0.0;
    req.state = 0;
    req.phaseSettingWord[0] = '\0';
    req.optBit = 0;
    req.unitAddressWord = 0;
    
    req.PhaseShiftCmdString[0] = '\0'; //the output of cmd build
    
    req.phaseShiftCmd = 0; //the string we've build doesn't contain the real command, we need to pack it into binary req.phasShiftCmd
    req.phaseShiftCmdCrc = // NULL;  so we can compute and append the crc as last bit of tx. Stm32 SPI1 supports "CRC" mode
    
    req.rc = Err_Ok;  // a phase shift cmd of all 0 (L)
                    // creates a shift request of 0 relative to the reference phase
    return req;