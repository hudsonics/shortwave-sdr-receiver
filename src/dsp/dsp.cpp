/*
 * dsp.cpp
 * =======
 *
 * DSP module - handles demodulation, filtering and AGC
 * - All demodulation modes use fixed-point arithmetic for efficiency on RX231 MCU
 * - Single IIR filter state is shared across modes - reset on mode/width change
 * - AGC updates DA0 every 38400 samples (~0.8s window) using peak detection
 * - dsp_process() is the C compatible ISR entry point, simply calls to DSP::process() which can access class members
 *
 * IIR filter implementation:
 *   y[n] = alpha * x[n] + (1 - alpha) * y[n - 1]
 *   Alpha values are scaled by 1024 and stored in filter_alpha[mode][width]
 *   Higher alpha = wider bandwidth = faster response
 */
 
#include "dsp.h"
#include "globals.h"

extern "C" {
    #include "r_smc_entry.h"
}

static const unsigned int filter_widths[5][3] = {
    { 3000u,  6000u, 9000u },       // AM
    { 1600u,  2400u, 3200u },       // USB
    { 1600u,  2400u, 3200u },       // LSB
    {  200u,   500u,  800u },       // CW
    { 5000u, 10000u,15000u }        // FM
};

char *mode_strings[] = { "AM", "USB", "LSB", "CW", "FM" };

// DC bias - output of baseband IQ filter/amp is 2.5V
#define DC_BIAS 3102

// IIR alpha values scaled by 1024, indexed [mode][filter_width]
// TO BE TUNED!
static const int32_t filter_alpha[5][3] = {
    { 402, 724, 946 },      // AM:  3k, 6k, 9k
    { 221, 321, 420 },      // USB: 1.6k, 2.4k, 3.2k
    { 221, 321, 420 },      // LSB: 1.6k, 2.4k, 3.2k
    {  27,	67, 107 },      // CW:  200, 500, 800
    { 607, 946,1016 }       // FM:  5k, 10k, 15k
};

// Apply IIR lowpass filter
int32_t DSP::apply_filter(int32_t input, uint8_t mode, uint8_t width) {
    int32_t alpha = filter_alpha[mode][width];

    filter_state = (alpha * input + (1024 - alpha) * filter_state) >> 10;
    return(filter_state);
}

// Noise reduction - moving average over NR_SIZE samples
#define NR_SIZE 4
static uint16_t nr_buffer[NR_SIZE] = { 2048, 2048, 2048, 2048 };
static uint8_t  nr_index = 0;
static uint32_t nr_sum   = 2048 * NR_SIZE;

uint16_t DSP::noise_reduce(uint16_t input) {
    nr_sum -= nr_buffer[nr_index];
    nr_buffer[nr_index] = input;
    nr_sum += input;
    nr_index = (nr_index + 1) % NR_SIZE;
    return (uint16_t)(nr_sum / NR_SIZE);
}

// AM demodulation
int32_t DSP::process_am(int32_t i, int32_t q) {
    // AM magnitude approximation using the formula:
    // |V| ˜ (983 * max + 407 * min) >> 10
    // This avoids sqrt() while giving <4% error vs true magnitude
    int32_t ai	= i < 0 ? -i : i;
    int32_t aq	= q < 0 ? -q : q;
    int32_t mx	= ai > aq ? ai : aq;
    int32_t mn	= ai < aq ? ai : aq;
    int32_t mag = (983 * mx + 407 * mn) >> 10;

    // Lowpass filter
    int32_t filtered = apply_filter(mag, AM, filter_width);

    int32_t out = filtered + 2048;

    return(out);
}

// USB demodulation
// VFO is offset -1500Hz so signal is already at baseband
// USB: output = I + Q (phasing method)
int32_t DSP::process_usb(int32_t i, int32_t q) {
    int32_t audio = (i + q) >> 1;

    int32_t filtered = apply_filter(audio, USB, filter_width);

    int32_t out = filtered + 2048;

    return(out);
}

// LSB demodulation
// VFO is offset +1500Hz so signal is already at baseband
// LSB: output = I - Q (phasing method)
int32_t DSP::process_lsb(int32_t i, int32_t q) {
    int32_t audio = (i - q) >> 1;

    int32_t filtered = apply_filter(audio, LSB, filter_width);

    int32_t out = filtered + 2048;

    return(out);
}

// CW demodulation
// VFO offset -700Hz so CW carrier beats at 700Hz
// Same as USB but with very narrow filter
int32_t DSP::process_cw(int32_t i, int32_t q) {
    int32_t audio = (i + q) >> 1;

    int32_t filtered = apply_filter(audio, CW, filter_width);

    int32_t out = filtered + 2048;

    return(out);
}

// FM demodulation
// Uses phase difference between consecutive samples
// output = (Q * I_prev - I * Q_prev) / (I^2 + Q^2)
// Magnitude normalisation approximated to avoid division
int32_t DSP::process_fm(int32_t i, int32_t q) {
    // Phase discriminator
    int32_t disc = (q * fm_i_prev - i * fm_q_prev) >> 10;

    // Normalise by magnitude squared approximation
    int32_t mag_sq = (fm_i_prev * fm_i_prev + fm_q_prev * fm_q_prev) >> 10;

    if (mag_sq < 1) {
        mag_sq = 1;         // prevent divide by zero
    }
    int32_t audio = (disc << 10) / mag_sq;

    // Store previous samples
    fm_i_prev	= i;
    fm_q_prev	= q;

    // Lowpass filter
    int32_t filtered = apply_filter(audio, FM, filter_width);

    // Scale and centre
    //int32_t out = (filtered * 8) + 2048;
    int32_t out = (filtered >> 4) + 2048;
    return(out);
}

// Main DSP processing function
uint16_t DSP::process(uint16_t i_raw, uint16_t q_raw) {
    // TODO: check effect of dynamic DC bias removal - could be contributing to Rx audio quality if too fast?
    int32_t i	= (int32_t)i_raw - measured_dc_i;
    int32_t q = ((int32_t)q_raw - measured_dc_q) * Q_GAIN_CORRECTION; // compensate Q LO amplitude

    // AGC peak detection on raw ADC values BEFORE digital gain is applied
    int32_t i_abs = i < 0 ? -i : i;
    int32_t q_abs = q < 0 ? -q : q;
    int32_t raw_level = i_abs > q_abs ? i_abs : q_abs;
    
    if ((uint16_t)raw_level > agc_peak) {
        agc_peak = (uint16_t)raw_level;         // instant attack
    } else {
        agc_peak = agc_peak - (agc_peak >> 8);  // slow decay every sample
    }
    
    i = i * DIGITAL_GAIN;
    q = q * DIGITAL_GAIN;
    
    // TODO: balance IQ signals - Q LO amplitude is lower than I LO - check Rx integration test results.

    if (i > 2047) {
        i = 2047;
    }
    if (i < -2048) {
        i = -2048;
    }
    if (q > 2047) {
        q = 2047;
    }
    if (q < -2048) {
        q = -2048;
    }

    int32_t demod_output;
    switch (mode) {
    case AM:  demod_output = process_am(i, q);  break;

    case USB: demod_output = process_usb(i, q); break;

    case LSB: demod_output = process_lsb(i, q); break;

    case CW:  demod_output = process_cw(i, q);  break;

    case FM:  demod_output = process_fm(i, q);  break;

    default:  demod_output = i + 2048;           break;
    }

    if (demod_output < 0) {
        demod_output = 0;
    }
    if (demod_output > 4095) {
        demod_output = 4095;
    }
    uint16_t output = (uint16_t)demod_output;

    agc_counter++;
    // 4800 samples at 48kHz = ~0.1s (10Hz) AGC window
    // Peak is tracked every sample, DA0 updated once per window
    if (agc_counter >= 4800) {
        agc_counter = 0;
        if (agc_peak > AGC_TARGET) {
            // Fast attack - large steps down
            uint16_t error = agc_peak - AGC_TARGET;
            uint16_t step = error >> 3;      // Keep proportional to error
            if (step < 1) {
	        step = 1;
	    }
            if (agc_voltage > AGC_MIN_VOLTAGE + step) {
                agc_voltage -= step;
	    } else {
                agc_voltage = AGC_MIN_VOLTAGE;
	    }
        } else {
            // Slow decay - small fixed steps up
            if (agc_voltage < AGC_MAX_VOLTAGE - AGC_DECAY) {
                agc_voltage += AGC_DECAY;
	    } else {
                 agc_voltage = AGC_MAX_VOLTAGE;
            }
        }

        R_Config_DA0_Set_ConversionValue(agc_voltage);
        agc_peak = 0;
    }
    output = noise_reduce(output);
    return(output);
}

// C-compatible wrapper to be called in A/D interrupt.
extern "C" uint16_t dsp_process(uint16_t i_raw, uint16_t q_raw) {
    return(dsp.process(i_raw, q_raw));
}

// DSP class methods
void DSP::init() {
    mode = AM;
    filter_width = 1;
    filter_state = 0;
    fm_i_prev = 0;
    fm_q_prev = 0;
    agc_peak = 0;
    agc_counter = 0;
    agc_voltage = AGC_INITIAL_VOLTAGE;
    R_Config_DA0_Set_ConversionValue(agc_voltage);
    nr_index = 0;
    
    nr_sum = 2048 * NR_SIZE;
    for (uint8_t i = 0; i < NR_SIZE; i++) {
        nr_buffer[i] = 2048;
    }
}

void DSP::set_mode(int8_t rotations) {
    int8_t new_mode = (int8_t)mode + (int8_t)rotations;

    if (new_mode < 0) {
        new_mode = 0;
    } else if (new_mode > 4) {
        new_mode = 4;
    }

    mode = (uint8_t)new_mode;

    // Reset filter state and FM memory on mode change
    filter_state	= 0;
    fm_i_prev		= 0;
    fm_q_prev		= 0;
}

uint8_t DSP::get_mode() {
    return(mode);
}

char * DSP::get_mode_string() {
    return(mode_strings[mode]);
}

void DSP::set_filter_width(int8_t rotations) {
    int8_t new_filter_width = (int8_t)filter_width + (int8_t)rotations;

    if (new_filter_width < 0) {
        new_filter_width = 0;
    } else if (new_filter_width > 2) {
        new_filter_width = 2;
    }

    filter_width = (uint8_t)new_filter_width;
    filter_state = 0;  // Reset filter state on width change
}

uint8_t DSP::get_filter_width() {
    return(filter_width);
}

char * DSP::get_filter_width_string() {
    char * const s[] = { "NARROW", "NORMAL", "WIDE" };

    return(s[filter_width]);
}

uint16_t DSP::get_filter_width_hz() {
    if (mode < 5 && filter_width < 3) {
        return(filter_widths[mode][filter_width]);
    }
    return(0);
}

uint16_t DSP::get_agc_voltage() {
    return(agc_voltage);
}
