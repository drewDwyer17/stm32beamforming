// /*
// This is a header file to contain the functions we are going to use to generate the 13 bit binary command
// that will be sent serial from our MCU to the phase shifting chip (...), using a SPI communication interface. 
// 

#ifndef PHASESHIFTER_H
#define PHASESHIFTER_H

#include <stdint.h>
#include <stdbool.h>

#define PHASESHIFTER_NUM_STATES          256u //maximum value 8 bits can take is 255 (11111111 in binary)
#define PHASESHIFTER_MAX_PHASE_SHIFT_DEG 360.0
#define PHASESHIFTER_MAX_PHASE_SET_WORD  255u
#define numStatesPerDegPhaseRotation (PHASESHIFTER_NUM_STATES / PHASESHIFTER_MAX_PHASE_SHIFT_DEG) // number of states per degree of phase shift
// // #define PHASESHIFTER_MAX_UNIT_ADDRESS          15u //

typedef enum lowToHigh
{
    LOW  = 0,
    HIGH = 1
} lowToHigh_t;


// //define a structure to keep track of all the components required to format a command to the phase shifter, given a requested phase shift in degrees.
// //the members will be used later to format and store the cmd in the phaesShiftCmd structrue for the request instance. 
typedef struct phaseShiftRequest_t
{
    double   requestedPhaseShift_deg;
    uint16_t numStates; // 2^8 = 256, always 256 for this device
    uint8_t  phaseSettingWord; //8 bits. Calculated from the requested phaes shift in degrees. 
    bool     optBit; //each cmd takes opt bit
    uint8_t  unitAddressWord; // 4-bits 
    uint16_t phaseShiftCmd; //a binary command of 13 bits, to be populated with helper functions using the other struct members. 

} phaseShiftRequest_t;

//static so that its only defined once, but can be used in multiple files. inline to speed up execution since its a small function.
//initialize the phase shift request to have 0 affect (shift) on the reference signal
static inline void phaseShiftRequestInitialize(phaseShiftRequest_t *req)
{
    req->requestedPhaseShift_deg = 0.0;
    req->numStates = PHASESHIFTER_NUM_STATES;
    req->phaseSettingWord = 0;
    req->optBit = LOW;
    req->unitAddressWord = 0;
    req->phaseShiftCmd = 0;
}

//Main API function to make a request for a phase shift command to be sent to the phase shifter circuit via SPI. 
phaseShiftRequest_t CreatePhaseShiftRequest(double requestedPhaseShift_deg,
                                   uint8_t unitAddressWord,
                                   bool optBit);
bool ClearPhaseShiftRequest(phaseShiftRequest_t *req);


//helpers for use in cmd generation from request

// //calculate the phase Setting word from the requested Phase shift. this is going to perform a division to find the state 
// associated with teh requested degree, and then to convert that sate into binary to get the 8 bit phase setting word.
uint8_t GetPhaseSettingWordFromDeg(double requestedPhaseShift_deg); //8 bit word

int16_t ConvertStateToBinary(int16_t reqState); //conversion of the reqState from int to binary after reqState has been calculated from requestedPhaseShift_deg

uint16_t SetPhaseShiftSCmd(phaseShiftRequest_t *req); //setter used to generate the cmd from a filled request. 
uint16_t GetPhaseShiftCmd(const phaseShiftRequest_t *req); //getter used to retrieve the command so we can send it serially to the Phase shifter. 

#endif // PHASESHIFTER_H