#include <include/ProcessHostCmds.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <include/Vga.h> 
#include <include/PhaseShifter.h>
#include <include/HostUart.h>

/*
 * Route the extracted command string to the correct parser.
 * The ID field determines which parser should process the command.
 */
host_uart_rc_t  processExtractedCmdStr(char *extractedCmdLine,
                                                processedHostCmd_t* processedCmd)
{
    uint8_t parsedCommandId = 0u;
    memset(processedCmd, 0, sizeof(*processedCmd));  //first clear any remenants from a previous command 

    if (sscanf(extractedCmdLine, "ID.%hhu", &parsedCommandId) != 1) {  //next, determine command type 
        return INVALID_COMMAND_ID;
    }
    processedCmd->CommandId = (hostCmdId_t)parsedCommandId;

    switch (processedCmd->CommandId) {
    case HOST_CMD_VGA: { 
        /* VGA command expected format: ID.0:ATT.23
        *attenuation passed by host as a character, but we want it as an integer. extract the char, interpret as int type, and then switch to supported attenuation cmd
        *only the supported values can be processed 
        */
        uint8_t idValue = 0u;
        uint8_t attValue = 0u;
        if (sscanf(extractedCmdLine, "ID.%hhu:ATT.%hhu", &idValue, &attValue) != 2) { 
            return MISSING_OR_INVALID_ATTRIBUTE;
        }
        switch (attValue) {
        case 0:
            processedCmd->att = ATTEN_0DB;
            break;
        case 1:
            processedCmd->att = ATTEN_1DB;
            break;
        case 2:
            processedCmd->att = ATTEN_2DB;
            break;
        case 4:
            processedCmd->att = ATTEN_4DB;
            break;
        case 8:
            processedCmd->att = ATTEN_8DB;
            break;
        case 16:
            processedCmd->att = ATTEN_16DB;
            break;
        case 22:
            processedCmd->att = ATTEN_22DB;
            break;
        case 23:
            processedCmd->att = ATTEN_23DB;
            break;
        default:
            return MISSING_OR_INVALID_ATTRIBUTE;
        }
        //for unused attributes, store 0 in the extern struct
        processedCmd->deg = 0.0;
        processedCmd->optBit = 0u;
        processedCmd->unit = 0u;
        spi_send(SPI1, processedCmd->att); //spisend(supportedAttenuationCmd_t)
        return OK;
    }

    case HOST_CMD_PS: { 
        //expected PS cmd format : ID.1:DEG.250:OPT.0:UNIT.12
        uint8_t dummyId = 0u;
        if (sscanf(extractedCmdLine,
                "ID.%hhu:DEG.%f:OPT.%hhu:UNIT.%hhu",
                &dummyId,
                &processedCmd->deg,
                &processedCmd->optBit,
                &processedCmd->unit) != 4)
        {
            return MISSING_OR_INVALID_ATTRIBUTE;
        }
        uint16_t commandPs = MakePSCommand(processedCmd->deg, processedCmd->optBit, processedCmd->unit);
        spi_send(SPI2, commandPs);
        return OK;
    }
    case HOST_CMD_COMBINED: 
        //expected cmd format:ID.2:ATT.23:DEG.250:OPT.0:UNIT.12
        uint8_t attValue = 0u;

        uint8_t dummyId = 0u;
        if (sscanf(extractedCmdLine,
                "ID.%hhu:ATT.%hhu:DEG.%f:OPT.%hhu:UNIT.%hhu",
                &dummyId,
                &attValue,
                &processedCmd->deg,
                &processedCmd->optBit,
                &processedCmd->unit) != 5)
        {
            return MISSING_OR_INVALID_ATTRIBUTE;
        }

        switch (attValue) {
        case 0:
            processedCmd->att = ATTEN_0DB;
            break;
        case 1:
            processedCmd->att = ATTEN_1DB;
            break;
        case 2:
            processedCmd->att = ATTEN_2DB;
            break;
        case 4:
            processedCmd->att = ATTEN_4DB;
            break;
        case 8:
            processedCmd->att = ATTEN_8DB;
            break;
        case 16:
            processedCmd->att = ATTEN_16DB;
            break;
        case 22:
            processedCmd->att = ATTEN_22DB;
            break;
        case 23:
            processedCmd->att = ATTEN_23DB;
            break;
        default:
            return MISSING_OR_INVALID_ATTRIBUTE;
        }

        uint16_t psCmd = MakePSCommand(processedCmd->deg, processedCmd->optBit, processedCmd->unit);
        spi_send(SPI2, psCmd);
        spi_send(SPI1, processedCmd->att); //vga cmd = supportedAttenuationCmd_t
        return OK;
    default:
        return INVALID_COMMAND_ID;
}
}

/*
 * The main function we use to process from USART1
 *Process: 
1. Extract a command string from the ring buffer. This is the raw data received from USART1, which is flushed to the ring buffer by the USART1 IDLE line interrupt. The command string is extracted until a newline character is reached, which marks the end of a command.
2. Route the command string to the correct handler based on the command ID. The command ID is the first part of the command string and determines which parser should be used to interpret the rest of the command.
3. Each handler parses the command string according to the expected format for that command ID and populate a structure with the variables in their proper types
 */
//find the inverse way to use rc later
host_uart_rc_t process_host_cmd(uint8_t *process_buf_ptr, char *extractedCmdLine, processedHostCmd_t *processedCmd)
{
    if (process_buf_ptr == NULL)
    {
        return INTERNAL_ERROR;
    }

    uint8_t *buf_start = process_buf_ptr;
    uint16_t i = 0; 
    while ((char)*process_buf_ptr != '\n' && i < (DMA_MAX_RX_CMD_LENGTH - 1u)) { 
        extractedCmdLine[i] = (char)*process_buf_ptr;
        process_buf_ptr++;//advances by size of uint8_t, which is 1 byte, so it moves to the next byte in the buffer
        i++;
    }

    if ((char)*process_buf_ptr != '\n' && i == (DMA_MAX_RX_CMD_LENGTH - 1u)) {
        return MISSING_TERMINATOR;
    }

    extractedCmdLine[i] = '\0';

    //now clear the process buffer 
    memset(buf_start, 0, DMA_MAX_RX_CMD_LENGTH);
    return processExtractedCmdStr(extractedCmdLine, processedCmd); 
}