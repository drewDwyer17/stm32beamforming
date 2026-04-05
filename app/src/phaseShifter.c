#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PE48820B_NUMSTATES 256u
#define PE48820B_MAX_PHASE_SHIFT_DEG 360.0
#define numStatesPerDegPhaseRotation (PE48820B_NUMSTATES / PE48820B_MAX_PHASE_SHIFT_DEG)


// Easier way of doing it would just be  (the most simple working method) : 
static uint16_t CreatePhaseRequest(double requestedShift_deg, bool optBit, uint8_t unitAddressWord)
{

    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);
    
    int fullcommand = (phaseSetWord << 5) | (optBit << 4) | unitAddressWord;

    return fullcommand;
} 


static uint16_t CreatePhaseRequestWithMMasks(double requestedShift_deg, bool optBit, uint8_t unitAddressWord)
{
    uint16_t phaseSetWord = (uint16_t)lround(requestedShift_deg * numStatesPerDegPhaseRotation);

    uint16_t fullcommand =
        ((phaseSetWord & 0xFFu) << 5) |
        (((uint16_t)optBit & 0x1u) << 4) |
        (unitAddressWord & 0x0Fu);

    return fullcommand;
}