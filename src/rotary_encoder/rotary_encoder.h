/*
 * rotary_encoder.h
 * ================
 *
 * Rotary encoder module using a finite state machine (FSM)
 * - Decodes quadrature encoder signals on CLK (P31) and DATA (P32)
 * - Rotary encoder push switch on P33
 * - FSM debounces encoder (only accepts valid state transitions) and detects direction of rotation
 * - on_interrupt() is called from IRQ1/IRQ2 interrupt handlers
 * - Rotation count is accumulated in rotations and consumed by main loop
 *
 * Event flags (stored in upper 4 bits of FSM state):
 * - ROT_CW:  clockwise rotation detected
 * - ROT_CCW: counter-clockwise rotation detected
 *
 * Derived from Rotary Arduino library written by Brian Low: https://github.com/brianlow/Rotary
 */
 
#ifndef rotary_encoder_h
#define rotary_encoder_h
#include <stdint.h>

// Encode event flags for clockwise turn and counter-clockwise rotations
#define ROT_CW   0x10
#define ROT_CCW  0x20

class RotaryEncoder {
    public:
        void init();
        uint8_t read_enc();
        uint8_t switch_closed();
        void set_rotations(int8_t);
        int8_t get_rotations();
        void on_interrupt();

    private:
        uint8_t persistent_state;
        volatile int8_t rotations;
};

#endif
