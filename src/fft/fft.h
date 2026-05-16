/*
 * fft.h
 * =====
 *
 * FFT spectrum waterfall display module
 * - 128-point complex FFT using fix_fft library (integer arithmetic)
 * - IQ sample buffers filled by S12AD0 ISR (Config_S12AD0_user.c), processed in main loop when fft_ready set
 * - fft_ready flag coordinates buffer handoff between ISR and main loop
 *
 * Frequency mapping (128 bins, 48kHz sample rate = 375Hz per bin):
 * - Bins   0-62: positive frequencies (0 to +24kHz) ? display pixels 63-125
 * - Bins 65-127: negative frequencies (-24kHz to 0)  ? display pixels  0-62
 * - Bin 63-64:   DC, which is discarded
 */
 
#ifndef fft_h
#define fft_h

#include <stdint.h>

#define FFT_SIZE	128
#define FFT_LOG2	7

#ifdef __cplusplus
class FFT {
    public:
        void process(uint8_t *magnitudes);

    private:
        uint8_t scale_magnitude(uint8_t mag);
};
#endif // __cplusplus

// C-compatible declarations for ISR access
extern volatile int16_t i_fft[FFT_SIZE];
extern volatile int16_t q_fft[FFT_SIZE];
extern volatile uint8_t fft_ready;

#endif // fft_h
