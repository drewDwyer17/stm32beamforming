#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define PE48820B_NUMSTATES 256u //maximum value 8 bits can take is 255 (11111111 in binary)
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
// #define PHASESHIFTER_MAX_PHASE_SET_WORD  255u
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG) // number of states per degree of phase shift
// // // #define PHASESHIFTER_MAX_UNIT_ADDRESS          15u //


//might not need all of these
typedef enum {
    Err_Ok, //processing path
    // Err_PShiftWord,
    // Err_OptBit,
    Err_NullPtr,
    // Err_CmdChksum, 
    // // // Err_SpiCRC,
    // Err_UnitAddWord,
    Err_NoSuchState,
    Err_PSResponse, 
    Err_ShiftOutOfRange,
    // Err_Count
} PhaseShiftMgr_Error_t;

//helper function to convert the phaseState to a string for printing LSB first
static inline void ConvertBinaryToASCII(uint16_t phaseSetWord, char *out)
{
    for (int i = 0; i < 8; i++) { 
        out[i] = (phaseSetWord & (1u << i)) ? '1' : '0';
    }
    out[8] = '\0';
}

typedef struct PackedPhaseShiftCmd_t {
    char cmdString[14];  //create cmd string of binary chars
    uint16_t packedCmdToSend;   // pack into int to send over SPI1
    // uint16_t packedCmdChksum; 
} PackedPhaseShiftCmd_t;

//structure to keep all the members required to create a command to be sent to the phaseShifter
//command structure used is defined in the PS datasheet:
 //https://www.mouser.ca/ProductDetail/pSemi/PE44820B-X?qs=Cb2nCFKsA8prCrkfl5FDIQ%3D%3D

// 13 bits for the full phase shift cmd 
//of structure [PhaseSetWordStr][optBit][unitAddressWord]
//e.g, program Word (LSB→MSB):  010010010 + 1100, OPT bit is synchronized to 90° bit 
typedef struct PhaseShiftRequest_t {
    double requestedShift_deg;
    uint16_t phaseSetWord; //calculated from shift request
    char PhaseSetWordStr[9];
    bool optBit;  //1 bit
    uint8_t unitAddressWord; //4 bits
    PackedPhaseShiftCmd_t packedPhaseShiftCmd; //a structure that contains full command to send via spi 
    uint16_t phaseShifterResponse;
    PhaseShiftMgr_Error_t rc; 

} PhaseShiftRequest_t;

//for the first time in the loop, initialize a new structure. 
// creates a shift request of all 0 (L) relative to the reference phase and returns the request structure 
static inline PhaseShiftRequest_t InitNewPhaseShiftRequest(void) 
{
    PhaseShiftRequest_t req;
    req.requestedShift_deg = 0.0;
    req.phaseSetWord = 0;
    req.PhaseSetWordStr[0] = '\0';
    req.optBit = 0;
    req.unitAddressWord = 0;
    req.packedPhaseShiftCmd.cmdString[0] = '\0';
    req.packedPhaseShiftCmd.packedCmdToSend =0;
    // req.packedPhaseShiftCmd.packedCmdChksum=0; 
    req.phaseShifterResponse = 0;

    req.rc = Err_Ok;  
    return req;
}

//for clearing/resetting a structure so we can reuse it 
static inline PhaseShiftMgr_Error_t ClearPhaseShiftRequest(PhaseShiftRequest_t *reqToClear)
{
    if (reqToClear == NULL) {
        return Err_NullPtr;
    }

    reqToClear->requestedShift_deg = 0.0;
    reqToClear->phaseSetWord = 0;
    reqToClear->PhaseSetWordStr[0] = '\0';
    reqToClear->optBit = 0;
    reqToClear->unitAddressWord = 0; 
    reqToClear->packedPhaseShiftCmd.cmdString[0] = '\0';
    reqToClear->packedPhaseShiftCmd.packedCmdToSend=0;
    // reqToClear->packedPhaseShiftCmd.packedCmdChksum=0; 
    reqToClear->phaseShifterResponse = 0;
    reqToClear->rc = Err_Ok;

    return reqToClear->rc; 
}

//a function used to build a phase shift command for a desired phase shift. 
// Will take the requested phase shift as an input. Returns the fully
// populated request. 

//this is a helper function that is called in the main API 'createPhaseShiftRequest" to pack the 
//provided inputs into a sendable command of the right format [phasesetword:8][opt:1][addrword:4]
static inline PhaseShiftMgr_Error_t BuildCommandfromRequest(PhaseShiftRequest_t *req)
{
    if (req->rc != Err_Ok) { return req->rc; }

    PackedPhaseShiftCmd_t newPackedCmd;

    newPackedCmd.cmdString[0] = '\0';
    newPackedCmd.packedCmdToSend = 0;
    // newPackedCmd.packedCmdChksum = 0;

    // pack into 13-bit command:
    // [state:8][opt:1][addr:4]
    newPackedCmd.packedCmdToSend =
        ((req->phaseSetWord & 0xFFu) << 5) | //clamp size then shift into place
        ((req->optBit ? 1u : 0u) << 4) | //make sure its 1 or 0 then shift into place
        (req->unitAddressWord & 0x0Fu);

    //now store the command string for the same request so we can debug
    for (int i = 0; i <13; i++) {
        newPackedCmd.cmdString[i] =
            (newPackedCmd.packedCmdToSend & (1u << i)) ? '1' : '0';
    }

    newPackedCmd.cmdString[13] = '\0';

    req->packedPhaseShiftCmd = newPackedCmd;

    return req->rc;
}

//main API to create a phase shift command given a requested shift relative to the reference signal phase
static inline PhaseShiftRequest_t SetPhaseShiftRequest(double _requestedShift_deg, bool optBit, uint8_t unitAddressWord, PhaseShiftRequest_t *req)
{
    if (req == NULL) {
        *req= InitNewPhaseShiftRequest(); 
    }

    PhaseShiftRequest_t *newReq = req;

    //clear it and restart if there's something wrong 
    if (newReq->rc != Err_Ok) {
        ClearPhaseShiftRequest(newReq);
    }

    //store the details of the request
    newReq->requestedShift_deg = _requestedShift_deg;
    newReq->optBit = optBit;
    newReq->unitAddressWord = unitAddressWord;

    //begin the calculation of the state. State is calculated from input degrees
    //from PE448 datasheet: 
    // 1. 205.3° × (256 states / 360°) = state 146
    // 2. convert to binary state 146  → 01001001
    // LSB→MSB (205.3 deg setting = 2.8° + 22.5° + 180°)
    if (_requestedShift_deg < 0.0 || _requestedShift_deg >= 360.0) {
        newReq->rc = Err_ShiftOutOfRange;
        return *newReq; //return early
    }

    //1. calculate the state
    newReq->phaseSetWord = (uint16_t)lround(_requestedShift_deg * numStatesPerDegPhaseRotation); //round up before truncating

    //max is 11111111
    if (newReq->phaseSetWord > 255u) {
        newReq->rc = Err_NoSuchState;
        return *newReq; //return early 
    }

    //convert phaseSetWord into an 8-bit binary string and store it starting at index 0 of PhaseSetWordStr
    ConvertBinaryToASCII(newReq->phaseSetWord, newReq->PhaseSetWordStr);

    newReq->rc = BuildCommandfromRequest(newReq);

    return *newReq;
}