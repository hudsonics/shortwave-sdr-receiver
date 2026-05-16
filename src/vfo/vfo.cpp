/*
 * vfo.cpp
 * =======
 *
 * VFO module - controls Si5351 CLK0 output via si5351 library
 * - Si5351 is initialised with 25MHz crystal and 8pF load capacitance (from ECS-250-8-36-RWN specs, see Y2 in schematic/BOM)
 * - CLK0 drive strength set to 2mA - TODO: try other values...
 * - Output is disabled during frequency changes to prevent glitching
 * - Frequency is multiplied by 400 when passed to si5351 libary since:
 * 	- CLK0 is used to generate IQ LOs via a quadrature divider which divides the input frequency by 4, and
 *	- The si5351 library expects frequency in hundredths of Hz
 */
 
#include "globals.h"
#include "si5351.h"
#include "vfo.h"

extern "C" {
#include "r_smc_entry.h"
}

// Lookup table for adjusting frequency.
static const uint32_t POW10[8] = {
    1U,         // 10^0
    10U,        // 10^1
    100U,       // 10^2
    1000U,      // 10^3
    10000U,     // 10^4
    100000U,    // 10^5
    1000000U,   // 10^6
    10000000U   // 10^7
};

void VFO::init() {
    frequency = HOME_FREQUENCY;
    osc.init(SI5351_CRYSTAL_LOAD_8PF, 25000000, 0);
    osc.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);

    // Set output frequency on init.
    osc.output_enable(SI5351_CLK0, 0);
    osc.set_freq(frequency * 400ULL, SI5351_CLK0);
    osc.output_enable(SI5351_CLK0, 1);
}

void VFO::set_frequency(int8_t rotations, uint8_t cursor_pos) {
    // Convert cursor position to frequency step size - takes number of rotations from rotary_encoder module
    // cursor_pos 0 = 10MHz digit, cursor_pos 7 = 1Hz digit
    // Radix maps cursor position to the corresponding power of 10 in lookup table
    uint8_t radix = 7 - cursor_pos; // 7 = max cursor position on freq
    uint32_t new_frequency = frequency + (rotations * POW10[radix]);

    if (new_frequency > MAX_FREQUENCY) {
        frequency = MAX_FREQUENCY;
    } else if (new_frequency < MIN_FREQUENCY) {
        frequency = MIN_FREQUENCY;
    } else {
        frequency = new_frequency;
    }

    // Apply LO offset so the signal of interest falls at baseband after mixing:
    // USB: LO 1500Hz below Rx frequency -> USB audio maps to 300-1800Hz
    // LSB: LO 1500Hz above Rx frequency -> LSB audio maps to 300-1800Hz
    // CW:  LO 700Hz below Rx frequency  -> CW beat note at 700Hz
    int16_t offset = 0;
    switch (dsp.get_mode()) {
    case USB:
        offset = -1500;
        break;

    case LSB:
        offset = +1500;
        break;

    case CW:
        offset = -700;
        break;

    default:
        offset = 0;
        break;
    }

    uint32_t lo_freq = frequency + offset;
    osc.output_enable(SI5351_CLK0, 0);
    osc.set_freq(lo_freq * 400ULL, SI5351_CLK0);
    osc.output_enable(SI5351_CLK0, 1);
}

char * VFO::get_frequency_string() {
    // Formats frequency as "XX.XXX.XXX MHz"
    // e.g. 14.15MHz -> "14.150.000 MHz"
    static char frequency_string[11];

    char digits[9]; // 8 digits + null terminator

    uint32_t f = frequency;

    for (int8_t i = 7; i >= 0; --i) {
        digits[i] = char('0' + (f % 10));
        f /= 10;
    }
    
    digits[8] = '\0';

    frequency_string[0]		= digits[0];
    frequency_string[1]		= digits[1];
    frequency_string[2]		= '.';
    frequency_string[3]		= digits[2];
    frequency_string[4]		= digits[3];
    frequency_string[5]		= digits[4];
    frequency_string[6]		= '.';
    frequency_string[7]		= digits[5];
    frequency_string[8]		= digits[6];
    frequency_string[9]		= digits[7];
    frequency_string[10]	= '\0';

    // Returns pointer to static string buffer - use before calling again!
    return(frequency_string);
}

uint32_t VFO::get_frequency() {
    return(frequency);
}
