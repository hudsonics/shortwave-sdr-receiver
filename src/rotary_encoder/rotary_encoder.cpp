/*
 * rotary_encoder.cpp
 * ==================
 *
 * Rotary encoder driver using a 6 state FSM to decode quadrature signals
 * - Encoder pins: CLK = P31 (IRQ1), DATA = P32 (IRQ2), SW = P33
 * - Both IRQ1 and IRQ2 trigger on_interrupt() on any edge
 * - FSM state is packed into a single byte:
 *     bits 3..0 = current state index (0..5)
 *     bits 5..4 = event flags (ROT_CW = 0x10, ROT_CCW = 0x20)
 * - Encoder is 2 pulses per detent - events fire only on ST_4->ST_0 (CW) and ST_5->ST_0 (CCW) transitions to give one event per detent click
 * - rotations is volatile as it is written from ISR and read from main loop
 */
 
#include "rotary_encoder.h"

extern "C" {
    #include "r_smc_entry.h"
}

// Masks - lower 4 bits holds the state index (0..5, bits 4..5 hold event flags (CW/CCW)
#define STATE_MASK 0x0F
#define EVENT_MASK 0x30

// FSM states (rows)
enum {
    ST_0 = 0,    // idle/reset aligned to CLK = 1, DATA = 1 (11)
    ST_1 = 1,    // intermediate
    ST_2 = 2,    // intermediate
    ST_3 = 3,    // hub
    ST_4 = 4,    // CW pre-detent
    ST_5 = 5     // CCW pre-detent
};

// 2-bit encoder line states (columns)
enum {
    ENC_00 = 0,    // CLK = 0, DATA = 0
    ENC_01 = 1,    // CLK = 0, DATA = 1
    ENC_10 = 2,    // CLK = 1, DATA = 0
    ENC_11 = 3     // CLK = 1, DATA = 1
};

// State transition table:
// Rows = current FSM state (low 4 bits of persistent_state)
// Cols = instantaneous encoder state (CLK << 1 | DATA)
// Each cell = (next_state & STATE_MASK) | optional ROT_CW/ROT_CCW flag
const uint8_t rot_enc_state_table[6][4] = {
    // ST_0: idle/reset - encoder at rest (CLK=1, DATA=1 - pulledup)
    {
        ST_3,       // 00 -> hub
        ST_2,       // 01 -> possible CW path
        ST_1,       // 10 -> possible CCW path
        ST_0        // 11 -> no change, stay idle
    },

    // ST_1: CLK falling edge first - could be start of CCW rotation
    {
        ST_3,       // 00 -> hub (no event - 2 pulse/detent encoder)
        ST_0,       // 01 -> invalid sequence, reset to ST_0
        ST_1,       // 10 -> no change from ST_0, wait
        ST_0        // 11 -> reset
    },

    // ST_2: DATA falling edge first - could be start of CW rotation
    {
        ST_3,       // 00 -> hub
        ST_2,       // 01 -> no change from ST_0, wait
        ST_0,       // 10 -> invalid sequence, reset
        ST_0        // 11 -> reset
    },

    // ST_3: hub - both signals low, rotation direction not yet confirmed
    {
        ST_3,           // 00 -> stay at hub
        ST_5,           // 01 -> commit to CCW track
        ST_4,           // 10 -> commit to CW track
        ST_0            // 10 -> back to idle
    },

    // ST_4: CW pre-detent - returning to idle via CW path
    {
        ST_3,           // 00 -> back to hub
        ST_3,           // 00 -> back to hub
        ST_4,           // 10 -> stay on CW track
        ST_0 | ROT_CW   // 11 -> detent reached, flag CW event
    },

    // ST_5: CCW pre-detent - returning to idle via CCW path
    {
        ST_3,           // 00 -> back to hub
        ST_5,           // 01 -> stay on CCW track
        ST_3,           // 10 -> back to hub
        ST_0 | ROT_CCW  // 11 -> detent reached, flag CCW event
    }
};

// Setup
void RotaryEncoder::init() {
    // Set initial state to 0.
    persistent_state = 0;
    rotations = 0;
}

uint8_t RotaryEncoder::read_enc() {
    // Read the instantaneous state of the encoder pins.
    // Pack clock into bit 1 and data into bit 0, yielding a 2-bit value 0..3:
    // enc_state = 0: CLK=0, DATA=0 (00)
    // enc_state = 1: CLK=0, DATA=1 (01)
    // enc_state = 2: CLK=1, DATA=0 (10)
    // enc_state = 3: CLK=1, DATA=1 (11)
    uint8_t enc_state = (PORT3.PIDR.BIT.B1 << 1) | PORT3.PIDR.BIT.B2;

    // Use the current FSM state's lower 4 bits as the row index (0..5).
    // Use enc_state (0..3) as the column index.
    // Table entry encodes:
    //   - next state's index in the lower 4 bits
    //   - possible CW (0x10) or CCW (0x20) flag in bits 4..5
    persistent_state = rot_enc_state_table[persistent_state & STATE_MASK][enc_state];

    // Return only the event bits (0x10 or 0x20) to signal a completed detent step.
    // 0x00 means no step event on this read.
    return(persistent_state & EVENT_MASK);
}

uint8_t RotaryEncoder::switch_closed() {
    // Get current switch state (P33)
    // 0 = open, 1 = closed (pulled up, so invert!)
    return(!PORT3.PIDR.BIT.B3);
}

void RotaryEncoder::on_interrupt(void) {
    uint8_t event = read_enc();

    if (event & ROT_CW) {
        set_rotations(get_rotations() - 1);
    }

    if (event & ROT_CCW) {
        set_rotations(get_rotations() + 1);
    }
}

void RotaryEncoder::set_rotations(int8_t new_rotations) {
    rotations = new_rotations;
}

int8_t RotaryEncoder::get_rotations() {
    return(rotations);
}
