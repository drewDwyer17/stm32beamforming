#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PE48820B_NUMSTATES 256u
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG)

typedef enum {
    Err_Ok,
    Err_NullPtr,
    Err_NoSuchState,
    Err_PSResponse,
    Err_ShiftOutOfRange,
} PhaseShiftMgr_Error_t;

// helper function to convert the phaseState to a string for printing LSB first
// static inline void ConvertBinaryToASCII(uint16_t phaseSetWord, volatile char *out)
// {
//     if (out == NULL) {
//         return;
//     }

//     for (int i = 0; i < 8; i++) {
//         out[i] = (phaseSetWord & (1u << i)) ? '1' : '0';
//     }
//     out[8] = '\0';
// }

typedef volatile struct PackedPhaseShiftCmd_t {
    // char cmdString[14];              // 13 bits + terminator
    uint16_t packedCmdToSend;        // packed 13-bit command stored in 16-bit container
} PackedPhaseShiftCmd_t;

// structure to keep all the members required to create a command to be sent to the phaseShifter
typedef struct PhaseShiftRequest_t {
    double requestedShift_deg;
    uint16_t phaseSetWord;                 // calculated from shift request
    // char PhaseSetWordStr[9];               // 8 bits + terminator
    bool optBit;                           // 1 bit
    uint16_t unitAddressWord;              // 4 bits used
    PackedPhaseShiftCmd_t packedPhaseShiftCmd;
    uint16_t phaseShifterResponse;
    PhaseShiftMgr_Error_t rc;
} PhaseShiftRequest_t;

// for the first time in the loop, initialize a new structure
static inline PhaseShiftMgr_Error_t InitNewPhaseShiftRequest(volatile PhaseShiftRequest_t *req)
{
    if (req == NULL) {
        return Err_NullPtr;
    }

    req->requestedShift_deg = 0.0;
    req->phaseSetWord = 0;
    // memset((void *)req->PhaseSetWordStr, 0, sizeof(req->PhaseSetWordStr));
    req->optBit = 0;
    req->unitAddressWord = 0;
    // memset((void *)req->packedPhaseShiftCmd.cmdString, 0, sizeof(req->packedPhaseShiftCmd.cmdString));
    req->packedPhaseShiftCmd.packedCmdToSend = 0;
    req->phaseShifterResponse = 0;
    req->rc = Err_Ok;

    return req->rc;
}

// for clearing/resetting a structure so we can reuse it
static inline PhaseShiftMgr_Error_t ClearPhaseShiftRequest(volatile PhaseShiftRequest_t *reqToClear)
{
    return InitNewPhaseShiftRequest(reqToClear);
}

// this is a helper function that is called in the main API to pack the
// provided inputs into a sendable command of the right format [phasesetword:8][opt:1][addrword:4]
static inline PhaseShiftMgr_Error_t BuildCommandfromRequest(volatile PhaseShiftRequest_t *req)
{
    if (req == NULL) {
        return Err_NullPtr;
    }

    if (req->rc != Err_Ok) {
        return req->rc;
    }

    PackedPhaseShiftCmd_t newPackedCmd;
    // memset((void *)newPackedCmd.cmdString, 0, sizeof(newPackedCmd.cmdString));
    newPackedCmd.packedCmdToSend = 0;

    // pack into 13-bit command:
    // [state:8][opt:1][addr:4]
    newPackedCmd.packedCmdToSend =
        ((req->phaseSetWord & 0xFFu) << 5) |
        (((uint16_t)req->optBit & 0x1u) << 4) |
        (req->unitAddressWord & 0x0Fu);

    // store command string for debug as LSB-first bit order
    // for (int i = 0; i < 13; i++) {
    //     newPackedCmd.cmdString[i] =
    //         (newPackedCmd.packedCmdToSend & (1u << i)) ? '1' : '0';
    // }
    // newPackedCmd.cmdString[13] = '\0';

    req->packedPhaseShiftCmd = newPackedCmd;

    return req->rc;
}

// main API to create a phase shift command given a requested shift relative to the reference signal phase
static inline PhaseShiftMgr_Error_t SetPhaseShiftRequest(double _requestedShift_deg, bool optBit, uint16_t unitAddressWord, volatile PhaseShiftRequest_t *req)
{
    if (req == NULL) {
        return Err_NullPtr;
    }

    if (req->rc != Err_Ok) {
        ClearPhaseShiftRequest(req);
    }

    req->requestedShift_deg = _requestedShift_deg;
    req->optBit = optBit;
    req->unitAddressWord = unitAddressWord & 0x0Fu;

    if (_requestedShift_deg < 0.0 || _requestedShift_deg >= 360.0) {
        req->rc = Err_ShiftOutOfRange;
        return req->rc;
    }

    req->phaseSetWord = (uint16_t)lround(_requestedShift_deg * numStatesPerDegPhaseRotation);

    if (req->phaseSetWord > 255u) {
        req->rc = Err_NoSuchState;
        return req->rc;
    }

    // ConvertBinaryToASCII(req->phaseSetWord, req->PhaseSetWordStr);

    req->rc = BuildCommandfromRequest(req);
    return req->rc;
}