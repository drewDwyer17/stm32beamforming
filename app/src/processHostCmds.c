#include <include/ProcessHostCmds.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <include/Vga.h> 
#include <include/PhaseShifter.h>
#include <include/HostUart.h>
#include <include/beamformerCan.h>


extern volatile processedHostCmd_t processedHostCmd; 
extern uint8_t can_data_rx[8];

/*
Main function for processing uart host commands. 
//need to change the implementation to match the following format: 

Expecting command format: "[Attribute1ID]:[Attribute1Val1],[Attribute1Val2]...:[Attribute2ID].[Attribute2Value1],[Attribute2Value2...],[AttributeNID]:[AttributeNValue1].[AttributeNValue2...]\n"

Steps: 
recieve host command into uart_rx_process_buf, 
extract UART str from the buffer
parse the string according to the command ID (attribute = "ID") value (0-2)
Command ID 0: phase shift command, 
Command ID 1: VGA command,
Command ID 2: Combined Command (vga and ps)


populate the processedHostCmd struct with the parsed values
send the command to the appropriate peripheral (VGA or Phase Shifter) via SPI


Command ID 0: phase shift command, expects requested phase state for all four phase shifters, in order (PS1 first, PS2... etc)
Example Format: ID.0:PS_STATES.1,6,7,8 //PS_STATES map to phaseState_e enum indexes. 
- Attribute 1 : Command ID (0-2)
- Attribute 2 : Requested Phase State Enum Indexes for PS1-PS4 (state
-- Value 1: Rquested Phase State Enum Index for PS1 (stateWordTableIndex)
-- Value 2: Requested Phase State Enum Index for PS2 (stateWordTableIndex)
-- Value 3: Requested Phase State Enum Index for PS3 (stateWordTableIndex)
-- Value 4: Requested Phase State Enum Index for PS4 (stateWordTableIndex)

Command ID 1: VGA command, expects only attenuation level attribute (8 bit word, maps to one of SupportedAttenuationCommand_e)
Example Format: ID.1:ATT.1 //ATT maps to SupportedAttenuationCommand_e indexes
- Attribute 1  : Command ID (0-2)
- Attribute 2 : Vga attenuation level (0-23)

Command ID 2: Combined Command: 
Example Format: ID.2:PS_IDX.1,6,7,8:ATT.1 //PS_IDXs map to phaseState_e enum indexes, VGA_ATT maps to SupportedAttenuationCommand_e indexes
- Attribute 1 : Command ID (0-2)
- Attribute 2 : Requested Phase State Enum Indexes for PS1-PS4 (stateWordTableIndex)
-- Value 1: Rquested Phase State Enum Index for PS1 (stateWordTableIndex)
-- Value 2: Requested Phase State Enum Index for PS2 (stateWordTableIndex)
-- Value 3: Requested Phase State Enum Index for PS3 (stateWordTableIndex)
-- Value 4: Requested Phase State Enum Index for PS4 (stateWordTableIndex)
-Attribute 3 : VGA attenuation level (0-23)

*/
host_cmd_rc_e  processExtractedCmdStr_Uart(char *extractedCmdLine,
                                                processedHostCmd_t *processedHostCmd)
{
    uint8_t parsedCommandId = 0u;
    memset(processedHostCmd, 0, sizeof(*processedHostCmd));  //first clear any remenants from a previous command 

    if (sscanf(extractedCmdLine, "ID.%hhu", &parsedCommandId) != 1) {  //next, determine command type 
        return INVALID_COMMAND_ID;
    }
    processedHostCmd->CommandId = (hostCmdId_e)parsedCommandId;

    switch (processedHostCmd->CommandId) {
    case HOST_CMD_VGA: { 
        /* VGA command expected format: ID.0:ATT.23
        *attenuation passed by host as a character, but we want it as an integer. extract the char, interpret as int type, and then switch to supported attenuation cmd
        *only the supported values can be processed 
        */
        uint8_t commandID = 0u;
        uint8_t targetAttIdx[4] = {0};
        if (sscanf(extractedCmdLine, "ID.%hhu:ATT.%hhu,%hhu,%hhu,%hhu", &commandID, &targetAttIdx) != 5) { 
            return MISSING_OR_INVALID_ATTRIBUTE;
        }

        for (int i = 0;i<4; i++) { 
            switch (targetAttIdx[i]) {
            case 0:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)0;
                break;
            case 1:
                processedHostCmd->att[i] =(SupportedAttenuationCommand_e)1;
                break;
            case 2:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)2;
                break;
            case 3:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)3;
                break;
            case 4:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)4;
                break;
            case 5:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)5;
                break;
            case 6:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)6;
                break;
            case 7:
                processedHostCmd->att[i] = (SupportedAttenuationCommand_e)7;
                break;
            default:
                return MISSING_OR_INVALID_ATTRIBUTE;
            }
        //for unused attributes, store 0 in the extern struct
        processedHostCmd->requestedPhaseStates[1]= 0;
        processedHostCmd->requestedPhaseStates[2]= 0;
        processedHostCmd->requestedPhaseStates[3]= 0;
        processedHostCmd->requestedPhaseStates[4]= 0;
        processedHostCmd->optBit = 0u;
        processedHostCmd->unit = 0u;
        //chip select [i]
        spi_send(SPI1, processedHostCmd->att[i]);
        //chipselectclear [i]  //send out 4 VGA commands, 1 to each of 4 vgas 
        } 
        processedHostCmd->processingDone = true; //flag that processing is done
        return OK;
    }

    case HOST_CMD_PS: { 
        //expected PS cmd format : ID.1:DEG.250:OPT.0:UNIT.12
        uint8_t dummyId = 0u;
        if (sscanf(extractedCmdLine,
                "ID.%hhu:PS_STATES.%hhu,%hhu,%hhu,%hhu", //expecting 4 values, one for each phase shift
                &dummyId,
                &processedHostCmd->requestedPhaseStates[1],
                &processedHostCmd->requestedPhaseStates[2],
                &processedHostCmd->requestedPhaseStates[3],
                &processedHostCmd->requestedPhaseStates[4]));
        
        else{
            return MISSING_OR_INVALID_ATTRIBUTE;
        }

        uint16_t commandPs1 = MakePSCommand( &processedHostCmd->requestedPhaseStates[1],0b001); //first ps 
        uint16_t commandPs2= MakePSCommand( &processedHostCmd->requestedPhaseStates[2], 0b010); //first ps 
        uint16_t commandPs3= MakePSCommand( &processedHostCmd->requestedPhaseStates[3], 0b011); //first ps 
        uint16_t commandPs4= MakePSCommand( &processedHostCmd->requestedPhaseStates[4], 0b100); //first ps 

        spi_send(SPI2, commandPs4);
        spi_send(SPI2, commandPs3);
        spi_send(SPI2, commandPs2);
        spi_send(SPI2, commandPs1);
     
        processedHostCmd->att = {0}; //initialize array to 0  
        //for unused attributes, store 0 in the extern struct
        processedHostCmd->processingDone = true; //flag that processing is done
        return OK;
    }
}
//     case HOST_CMD_COMBINED: 
//         //expected cmd format:ID.2:ATT.23:DEG.250:OPT.0:UNIT.12
//         uint8_t targetAttIdx = 0u;

//         uint8_t dummyId = 0u;
//         uint8_t dummyId = 0u;
//         if (sscanf(extractedCmdLine,
//                 "ID.%hhu:PS_STATES.%hhu,%hhu,%hhu,%hhu:ATT.%hhu", //expecting 4 values, one for each phase shift
//                 &dummyId,
//                 &processedHostCmd->requestedPhaseStates[1],
//                 &processedHostCmd->requestedPhaseStates[2],
//                 &processedHostCmd->requestedPhaseStates[3],
//                 &processedHostCmd->requestedPhaseStates[4],
//                 &targetAttIdx));
        
//         else{
//             return MISSING_OR_INVALID_ATTRIBUTE;
//         }


//         switch (targetAttIdx) {
//         case 0:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)0;
//             break;
//         case 1:
//             processedHostCmd->att =(SupportedAttenuationCommand_e)1;
//             break;
//         case 2:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)2;
//             break;
//         case 3:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)3;
//             break;
//         case 4:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)4;
//             break;
//         case 5:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)5;
//             break;
//         case 6:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)6;
//             break;
//         case 7:
//             processedHostCmd->att = (SupportedAttenuationCommand_e)7;
//             break;
//         default:
//             return MISSING_OR_INVALID_ATTRIBUTE;
//         }
//         uint16_t commandPs1 = MakePSCommand( &processedHostCmd->requestedPhaseStates[1], 0, 0b001); //first ps 
//         uint16_t commandPs2= MakePSCommand( &processedHostCmd->requestedPhaseStates[2], 0, 0b010); //first ps 
//         uint16_t commandPs3= MakePSCommand( &processedHostCmd->requestedPhaseStates[3], 0, 0b011); //first ps 
//         uint16_t commandPs4= MakePSCommand( &processedHostCmd->requestedPhaseStates[4], 0, 0b100); //first ps 

//         spi_send(SPI2, commandPs4);
//         spi_send(SPI2, commandPs3);
//         spi_send(SPI2, commandPs2);
//         spi_send(SPI2, commandPs1);
//         spi_send(SPI1, processedHostCmd->att); //vga cmd = supportedAttenuationCmd_t
//         processedHostCmd->processingDone = true; //flag that processing is done
//         return OK;
//     default:
//         return INVALID_COMMAND_ID;
//     }
// }

host_cmd_rc_e process_host_cmd_uart(uint8_t *process_buf_ptr, char *extractedCmdLine, processedHostCmd_t *processedHostCmd)
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
    return processExtractedCmdStr_Uart(extractedCmdLine, processedHostCmd); 
}

//except can DATA payload of 8 bytes if CAN message Identifier passes acceptance filters set up in 
//canSetUpAcceptanceFilters(CanTargetMcuMask_e canMCUId_Target)
host_cmd_rc_e process_host_cmd_can(uint8_t *can_data_rx) {

    if (!processedHostCmd.processingDone) {
        //we are waiting for previous command processing to finish
        return LAST_COMMAND_PROCESSING_NOT_FINISHED;
    }
    switch ((CanCommandId_e)can_data_rx[0]) {  

        case (CAN_CMD_PHASE_SHIFTERS): 
            uint8_t commandps1 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[1], 0b00000001); //expecting byte 1 is PS1 state  
            uint8_t commandps2 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[2], 0b00000010) ; //2
            uint8_t commandps3 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[3], 0b00000011) ; //3
            uint8_t commandps4 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[4], 0b00000100) ; //4
            spiSend(SPI2, commandps4); //send in reverse order because the first command will be pushed to the last phase shifter. 
            spiSend(SPI2, commandps3);  
            spiSend(SPI2, commandps2);
            spiSend(SPI2, commandps1);
            return OK;

        case ((CAN_CMD_VGA)): 
        /* 
        To elias to change the VGA command reception to better match the phase shfiter 
        */

        //Expected CAN data [8 bytes] format: [commandTypeId Byte 1 (0-2)][vgaAttenuationLevel Byte 2 (SupportedAttenuationCommand_e)][unused][unused][unused][unused][unused][unused]
        //note to self, expecting attenuation level to be 8 bits
         
            SupportedAttenuationCommand_e commandVga = (SupportedAttenuationCommand_e)&can_data_rx[2]; //just to check that the value is supported
            //if the cast fails, return error code
            // if ((commandVga != ATTEN_0DB) && (commandVga != ATTEN_1DB) && (commandVga != ATTEN_2DB) && (commandVga != ATTEN_4DB) && (commandVga != ATTEN_8DB) && (commandVga != ATTEN_16DB) && (commandVga != ATTEN_22DB) && (commandVga != ATTEN_23DB)) {
            //     return MISSING_OR_INVALID_ATTRIBUTE;
            // }
            spiSend(SPI1, commandVga);
            return OK;

        case (CAN_CMD_COMBINED):
        //Expected CAN data [8 bytes] format: [commandTypeId Byte 1 (0-2)][stateEnumIndex1 Byte 2 (stateWordTableIndex for PS1)][stateWordTableIndex for PS2 Byte 3][stateWordTableIndex for PS3 Byte 4][StateWordTableEnumIndex for PS4 Byte 5][vgaAttenuationLevel Byte 6 (SupportedAttenuationCommand_e)][unused][unused]
            uint8_t commandps1 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[1], 0b00000001); //expecting byte 1 is PS1 state  
            uint8_t commandps2 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[2], 0b00000010) ; //2
            uint8_t commandps3 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[3], 0b00000011) ; //3
            uint8_t commandps4 = MakePSCommand((optimizedPhaseState_e)&can_data_rx[4], 0b00000100) ; //4

            //change this SupportedAttenuationCommand_e commandVga = (SupportedAttenuationCommand_e)&can_data_rx[5];
            spiSend(SPI1, commandVga);
            spiSend(SPI2, commandps4); //send in reverse order because the first command will be pushed to the last phase shifter. 
            spiSend(SPI2, commandps3);  
            spiSend(SPI2, commandps2);
            spiSend(SPI2, commandps1);
            return OK;
    }
}
