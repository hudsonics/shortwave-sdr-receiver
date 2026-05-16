/*
 * fft.cpp
 * =======
 *
 * FFT processing and spectrum magnitude calculation for the waterfall display
 * - IQ sample buffers filled by S12AD0 interrupt, processed in main loop
 * - 128 point complex FFT using fix_fft library (integer arithmetic only)
 * - Magnitude approximation: max(|real|, |imag|) avoids sqrt()
 * - >> 2 scaling maps 16-bit FFT output to 8-bit display range
 * - Logarithmic scaling via scale_magnitude() gives better dynamic range
 * - Output range: 0-32 pixels (waterfall display height)
 *
 * Frequency mapping (128 bins, 48kHz sample rate = 375Hz per bin):
 * - Bins   1-63:  positive frequencies (+375Hz to +23.625kHz) ? pixels 63-125
 * - Bins  65-127: negative frequencies (-375Hz to -23.625kHz) ? pixels  0-62
 * - Bin 0:        DC component, zeroed
 * - Bin 64:       Nyquist frequency, zeroed
 *
 * Note: pixel order is reversed in draw_waterfall() to correct for frequency wrapping due to direct conversion
 */

#include "globals.h"
#include "fft.h"
#include "dsp.h"

extern "C" {
    #include "fix_fft.h"
    #include "r_smc_entry.h"
}
volatile int16_t i_fft[FFT_SIZE];
volatile int16_t q_fft[FFT_SIZE];
volatile uint8_t fft_ready = 0;

void FFT::process(uint8_t *magnitudes) {
    static int16_t fr[FFT_SIZE];
    static int16_t fi[FFT_SIZE];

    for (uint8_t i = 0; i < FFT_SIZE; i++) {
        fr[i]	= i_fft[i];
        fi[i]	= q_fft[i];
    }
    fft_ready = 0;
    
    // fix_fft expects int16_t input cast to char* to then scale output to full int16_t range
    fix_fft((char *)fr, (char *)fi, FFT_LOG2, 0);

    // Zero out DC and Nyquist bins
    fr[0] = 0;
    fi[0] = 0;
    fr[64] = 0;
    fi[64] = 0;

    for (uint8_t i = 0; i < 63; i++) {
        // Negative frequencies
        int16_t r16		= fr[65 + i];
        int16_t im16	= fi[65 + i];
        if (r16 < 0) {
            r16 = -r16;
        }
        if (im16 < 0) {
            im16 = -im16;
        }
        uint16_t mag = (uint16_t)(r16 > im16 ? r16 : im16);
        magnitudes[i] = scale_magnitude((uint8_t)(mag >> 2));

        // Positive frequencies
        r16 = fr[i + 1];
        im16 = fi[i + 1];
        if (r16 < 0) {
            r16 = -r16;
        }
        if (im16 < 0) {
            im16 = -im16;
        }
        mag = (uint16_t)(r16 > im16 ? r16 : im16);
        magnitudes[i + 63] = scale_magnitude((uint8_t)(mag >> 2));
    }
}

// Logarithmic lookup table mapping raw FFT magnitude (0-127) to display height (0-32)
// 	- Finer granularity at low magnitudes improves visibility of weak signals
uint8_t FFT::scale_magnitude(uint8_t mag) {
    if (mag == 0) {
        return(0);
    }
    if (mag < 2) {
        return(1);
    }
    if (mag < 4) {
        return(2);
    }
    if (mag < 6) {
        return(3);
    }
    if (mag < 8) {
        return(4);
    }
    if (mag < 10) {
        return(5);
    }
    if (mag < 13) {
        return(6);
    }
    if (mag < 16) {
        return(7);
    }
    if (mag < 19) {
        return(8);
    }
    if (mag < 22) {
        return(9);
    }
    if (mag < 26) {
        return(10);
    }
    if (mag < 30) {
        return(11);
    }
    if (mag < 34) {
        return(12);
    }
    if (mag < 38) {
        return(13);
    }
    if (mag < 43) {
        return(14);
    }
    if (mag < 48) {
        return(15);
    }
    if (mag < 53) {
        return(16);
    }
    if (mag < 58) {
        return(17);
    }
    if (mag < 64) {
        return(18);
    }
    if (mag < 70) {
        return(19);
    }
    if (mag < 76) {
        return(20);
    }
    if (mag < 83) {
        return(21);
    }
    if (mag < 90) {
        return(22);
    }
    if (mag < 97) {
        return(23);
    }
    if (mag < 104) {
        return(24);
    }
    if (mag < 111) {
        return(25);
    }
    if (mag < 118) {
        return(26);
    }
    if (mag < 122) {
        return(27);
    }
    if (mag < 125) {
        return(28);
    }
    if (mag < 127) {
        return(29);
    }
    if (mag < 128) {
        return(30);
    }
    return(32);
}
