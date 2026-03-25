// //
// This is a header file to contain the functions we are going to use to generate the 13 bit binary command
// that will be sent serial from our MCU to the phase shifting chip (...), using a SPI communication interface.
// //


//usage
// double requestedShift_deg = 33;
// BuildPhaseShiftCmd(requestedShift_deg, 1, 1, &shiftRequest);

//initializes the rc, etc, and the other parts
// BuildPhaseShiftCmd(requestShift_deg, optBit, &newReq) //use input degree to calculate a command. Populate the structure and then return it
//as the return of the function to the newReq that you initialized

// SendCmdAsSpi(newReq->phaseShiftCmd) {
//here we will use spi commands that we have defined to actually communicate the command created
// }

#ifndef PHASESHIFTER_H
#define PHASESHIFTER_H

#include <stdint.h>
#include <stdbool.h>

#define PE48820B_NUMSTATES 256u //maximum value 8 bits can take is 255 (11111111 in binary)
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
// #define PHASESHIFTER_MAX_PHASE_SET_WORD  255u
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG) // number of states per degree of phase shift
// // // #define PHASESHIFTER_MAX_UNIT_ADDRESS          15u //

// Second Draft

//start defining an error struct. Evenutally, might move this to its own other file containing other error RCs that we will define later
// in the project

//might not need all of these
typedef enum {
    Err_Ok, //happy path
    Err_PShiftWord,
    Err_OptBit,
    Err_CmdLength,
    Err_CmdChksum, //if length is off, the checksum is too. but length is quicker check? 
    Err_UnitAddWord,
    Err_NoSuchState,
    Err_ShiftOutOfRange,
    Err_Count
} PhaseShiftMgr_Error_t;

// } error_t

typedef struct PackedPhaseShiftCmd_t {
    uint16_t cmd;          // actual sendable command
    char cmdString[14];    // 13 bits + null terminator
} PackedPhaseShiftCmd_t;

//structure to keep all the members required to create a command interpretable by the phase shifter.
//command structure used is defined in the PS datasheet:
 https://www.mouser.ca/ProductDetail/pSemi/PE44820B-X?qs=Cb2nCFKsA8prCrkfl5FDIQ%3D%3D

typedef struct PhaseShiftRequest_t {
    double requestedShift_deg;
    uint16_t state; // 2^8 = 256, always 256 for this device
    char phaseSettingWord[9];
    bool optBit; // each cmd takes an opt bit
    uint8_t unitAddressWord; //4 bits
    uint16_t phaseShiftCmd; // 13 bits for the full, phase shift cmd, of structure [PhaseSettingWord][optBit][unitAddressWord]
    char phaseShiftCmdString[14]; // full 13 bit cmd as a string for printing
    PhaseShiftMgr_Error_t rc; //return code to see any errors during cmd population

} PhaseShiftRequest_t;

static inline PhaseShiftRequest_t InitNewPhaseShiftRequest(void)
{
    PhaseShiftRequest_t req;
    req.requestedShift_deg = 0.0;
    req.state = 0;
    req.phaseSettingWord[0] = '\0';
    req.optBit = 0;
    req.unitAddressWord = 0;
    req.phaseShiftCmd = 0;
    req.phaseShiftCmdString[0] = '\0';
    req.rc = Err_Ok;  // a phase shift cmd of all 0 (L)
                      // creates a shift request of 0 relative to the reference phase
    return req;
}

static inline void ResetPhaseShiftRequest(PhaseShiftRequest_t *reqToClear)
{
    if (reqToClear == NULL) {
        return;
    }

    reqToClear->requestedShift_deg = 0.0;
    reqToClear->state = 0;
    reqToClear->phaseSettingWord[0] = '\0';
    reqToClear->optBit = 0;
    reqToClear->unitAddressWord = 0;
    reqToClear->phaseShiftCmd = 0;
    reqToClear->phaseShiftCmdString[0] = '\0';
    reqToClear->rc = Err_Ok;
}

//a function used to build a phase shift command for a desired phaes shift. Will take the requested phase shift as an input. Returns the fully
//populated request. Validation is performed along the way. If at any point Err_ok changes to an error rc, we return from
//the function. immediately afterwards (in the main logic), implement a check on the rc stored in the structure before proceeding.
//e.g.:

static inline void ConvertToBinary(uint16_t state, char *out)
{
    for (int i = 7; i >= 0; i--) {
        out[7 - i] = (state & (1u << i)) ? '1' : '0';
    }
    out[8] = '\0';
}

static inline PackedPhaseShiftCmd_t PackPhaseShiftCmdFromString(const char *phaseSettingWord, bool optBit, uint8_t unitAddress)
{
    PackedPhaseShiftCmd_t result;
    uint16_t state = 0;

    result.cmd = 0;
    result.cmdString[0] = '\0';

    // convert "01001001" (LSB → MSB) → integer
    for (int i = 7; i >= 0; i--) {
        state <<= 1;
        if (phaseSettingWord[i] == '1') {
            state |= 1u;
        }
    }

    // pack into 13-bit command:
    // [state:8][opt:1][addr:4]
    result.cmd =
        ((state & 0xFFu) << 5) |
        ((optBit ? 1u : 0u) << 4) |
        (unitAddress & 0x0Fu);

    // build string version (MSB → LSB for readability)
    for (int i = 12; i >= 0; i--) {
        result.cmdString[12 - i] = (result.cmd & (1u << i)) ? '1' : '0';
    }
    result.cmdString[13] = '\0';

    return result;
}


//main API

static inline PhaseShiftRequest_t *BuildPhaseShiftCmd(double _requestedShift_deg, bool optBit, uint8_t unitAddress, PhaseShiftRequest_t *newReq)
{
    PackedPhaseShiftCmd_t packed;
    PhaseShiftRequest_t *req = newReq; //dereference so we can access and manipulate values

    if (req == NULL) {
        return NULL;
    }

    if (req->rc != Err_Ok) {
        ResetPhaseShiftRequest(req);
    }

    req->requestedShift_deg = _requestedShift_deg;
    req->optBit = optBit;
    req->unitAddressWord = unitAddress;

    //first calculate the state from the input degrees.
    // Example calculation from datasheet:
    //205.3° × (256 states / 360°) = state 146
    // state 146  → 01001001
    // LSB→MSB (205.3 deg setting = 2.8° + 22.5° + 180°)
    if (_requestedShift_deg < 0.0 || _requestedShift_deg >= 360.0) {
        req->rc = Err_ShiftOutOfRange;
        return req;
    }

    req->state = (uint16_t)(_requestedShift_deg * numStatesPerDegPhaseRotation);

    if (req->state > 255u) {
        req->rc = Err_NoSuchState;
        return req;
    }

    ConvertToBinary(req->state, req->phaseSettingWord);

    packed = PackPhaseShiftCmdFromString(
        req->phaseSettingWord,
        req->optBit,
        req->unitAddressWord
    );

    req->phaseShiftCmd = packed.cmd;

    for (int i = 0; i < 14; i++) {
        req->phaseShiftCmdString[i] = packed.cmdString[i];
    }

    req->rc = Err_Ok;

    return req;
}

#endif