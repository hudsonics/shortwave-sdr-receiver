/*
 * vfo.h
 * =====
 *
 * Variable Frequency Oscillator (VFO) module for Si5351 clock generator IC
 * - Controls receive frequency via I2C to Si5351 on CLK0
 * - Frequency range covers most of the HF shortwave band (2.3 - 30 MHz)
 * - Si5351 output frequency is multiplied by 400 internally to achieve the required LO frequency for the direct conversion mixer
 * - VFO offsets for SSB and CW modes are applied in vfo.cpp to account for the IF shift required for correct demodulation
 *
 * Frequency limits:
 * - HOME:  7.415 MHz (default on start)
 * - MIN:   2.3 MHz
 * - MAX:  30.0 MHz
 */
 
#include "si5351.h"

#ifndef vfo_h
#define vfo_h

#define HOME_FREQUENCY		7415000
#define MIN_FREQUENCY		2300000
#define MAX_FREQUENCY		30000000

class VFO {
    public:
        void init();
        void update();
        void set_frequency(int8_t, uint8_t);
        uint32_t get_frequency();
        char * get_frequency_string();

        Si5351 osc;
    private:
        uint32_t frequency;
};

#endif
