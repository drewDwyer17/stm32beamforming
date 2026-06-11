#ifndef PROCESSHOSTCMDS_H
#define PROCESSHOSTCMDS_H

#include <stdbool.h>
#include <stdint.h>
#include <include/PhaseShifter.h> 
#include <include/Vga.h> 

typedef enum host_uart_rc { 
    OK = 0, 
    INVALID_COMMAND_ID = 1,
    MISSING_OR_INVALID_ATTRIBUTE = 2, 
    CMD_LENGTH_INVALID = 3,
    MISSING_TERMINATOR = 4,
    INTERNAL_ERROR = 5
} host_uart_rc_t;

typedef enum hostCmdId { 
     HOST_CMD_VGA,
     HOST_CMD_PS,
     HOST_CMD_COMBINED
} hostCmdId_t;

/* 
Structure to be populated during the processing of commands sent serially from host uart. 

Uart frame structure : [startBit][8dataBits][stopBit]
Host command contained in data payload. 

Host Command Format: 
- 3 different command types, numerated by ID. ID used by parser to determine expected attributes. 
- Each payload structure contains: [commandId][attribute1ID.attribute1Value][attribute2ID.attribute2Value]...[attributeNID.attributeNValue]
- Attributes separated by ":" delimeter, values separated by ".". 

Command Type 0: VGA command
- ID: 0 
- Accepted Attributes: Attenuation Level ("ATT")
- Example Command Data Payload: "ID.0:ATT.23\n" set attenuation to 0dB

Command Type 1: Phase Shift Command 
- ID: 1 
- Accepted Attributes: Requested phase shift in deg ("DEG"), Unit adress, "UNIT", Opt Bit "OPT"
- Example Command Data Payload: "ID.1:DEG.90:UNIT.2:OPT.1\n" set unit 2 to 90 degree phase shift with opt bit on.

Command type 2: Combined VGA and Phase Shift Command
- ID: 2
- Accepted Attributes: Attenuation level ("ATT"), Requested phase shift in deg ("DEG"), Unit adress, "UNIT", Opt Bit "OPT"
- Example Command Data Payload: "ID.2:DEG.90:UNIT.2:OPT.1:ATT.23\n" set unit 2 to 90 degree phase shift with opt bit on and attenuation to 0dB.

During command parsing, hostCommand_t structure is populated with the values of the attributes. 
If an attribute is not expected given the command ID, it takes a default "NULL" value. 

*/
typedef struct {
    hostCmdId_t CommandId;
    SupportedAttenuationCommand_t att; //attenuation level, vga command attribute.
    float deg; //requested degrees, phase shift cmd attribute.
    uint8_t optBit; //opt bit, phase shift cmd attribute. 
    uint8_t unit; //unit address, phase shift cmd attribute.
} processedHostCmd_t;

extern volatile processedHostCmd_t processedHostCmd;

host_uart_rc_t process_host_cmd(uint8_t *buf, char *extractedCmdLine, processedHostCmd_t *processedCmd);

host_uart_rc_t  processExtractedCmdStr(char *extractedCmdLine,
                                                processedHostCmd_t *processedCmd);
#endif