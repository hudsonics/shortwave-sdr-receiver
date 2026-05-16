/*
 * globals.h
 * =========
 *
 * Global object declarations shared across all source files
 * - Single point of include for all module headers
 * - All objects are defined once in globals.cpp and declared extern here
 * - RotaryEncoder_ISR_Handler is exposed as extern "C" for use as in interrupt handler in Config_ICU_user.c
 */
 
#pragma once
#include "rotary_encoder.h"
#include "display.h"
#include "vfo.h"
#include "dsp.h"
#include "fft.h"

extern RotaryEncoder rotary_encoder;
extern Display display;
extern VFO vfo;
extern DSP dsp;
extern FFT fft;

#ifdef __cplusplus
extern "C" {
#endif
void RotaryEncoder_ISR_Handler(void);

#ifdef __cplusplus
}
#endif
