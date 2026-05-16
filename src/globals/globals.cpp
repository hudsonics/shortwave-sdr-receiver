/*
 * globals.cpp
 * ===========
 *
 * Global object definitions and hardware interrupt handler
 * - Defines the single instances of all module objects
 * - RotaryEncoder_ISR_Handler is called from both IRQ1 and IRQ2 interrupt vectors (Config_ICU_user.c) on any edge of the encoder CLK or DATA pins
 */
 
#include "globals.h"

RotaryEncoder rotary_encoder;
Display display;
VFO vfo;
DSP dsp;
FFT fft;

extern "C" void RotaryEncoder_ISR_Handler(void) {
    rotary_encoder.on_interrupt();
}
