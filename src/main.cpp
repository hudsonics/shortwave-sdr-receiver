/*
 * main.cpp
 * ========
 *
 * Main entry point and control loop for the SDR receiver
 * - Initialises all peripherals and modules in the correct order
 * - Main loop processes FFT buffers and handles rotary encoder input
 *
 * Initialisation order:
 * 1. System, GPIO, ICU (interrupts)
 * 2. DA0 (AGC CV), DA1 (audio output)
 * 3. S12AD0 (ADC), MTU0 (48kHz A/D sample timer)
 * 4. RIIC0 (I2C for display and Si5351 VFO)
 * 5. VFO, rotary encoder, DSP
 * 6. Display (requires delay for OLED stabilisation)
 *
 * Main loop:
 * - Runs once per FFT buffer fill (~2.67ms at 48kHz/128 samples)
 * - Display updates every 10th buffer (~267ms)
 * - Rotary encoder module tracks rotations on interrupt - any necassary changes to the user interface based on rotations are made when display is updated every ~276ms
 *
 * Note: abort() is defined - this is required by CC-RX runtime but never called in normal operation
 */

#include "globals.h"

extern "C" {
    #include "r_smc_entry.h"
    void abort(void);
}

void loop(void);

void abort(void) {}

void main(void) {
    SYSTEM.VBATTCR.BIT.VBATTDIS = 1; // Disable battery backup functionality - may lead to instability
    R_Systeminit();

    // Rotary encoder GPIO pins and interrupts
    R_Config_PORT_Create();
    R_Config_ICU_Create();
    R_Config_ICU_IRQ1_Start();
    R_Config_ICU_IRQ2_Start();

    // AGC D/A converter
    R_Config_DA_Create();
    R_Config_DA0_Start();   // DA0 = AGC CV

    // A/D converter
    R_Config_S12AD0_Create();
    R_Config_S12AD0_Start();

    // A/D sample rate timer - 48kHz (~20.833us)
    R_Config_MTU0_Create();
    R_Config_MTU0_Start();

    // I2C - interface with OLED display and Si5351 oscillator
    R_Config_RIIC0_Create();
    R_Config_RIIC0_Start();

    // Initialise peripherals and modules
    R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);
    vfo.init();
    rotary_encoder.init();
    dsp.init();
    R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
    display.init();
    
    // Audio output D/A converter
    R_Config_DA1_Start();

    while (1) {
        loop();
    }
}

void loop() {
    static uint16_t fft_update_counter = 0;
    if (fft_ready) {
        fft_update_counter++;
        if (fft_update_counter >= 10) {
            fft_update_counter = 0;

            uint8_t magnitudes[126];
            fft.process(magnitudes);

            uint8_t rotations = rotary_encoder.get_rotations();
            rotary_encoder.set_rotations(0);
	    // If rotary encoder has been rotated since last display update...
            if (rotations != 0) {
		// Change cursor position if rotary encoder push switch is closed
                if (rotary_encoder.switch_closed() == 1) {
                    display.set_cursor_pos(rotations);
                } else {
                    uint8_t current_cursor_pos = display.get_cursor_pos();
		    // Update frequency when cursor over frequency string
                    if (current_cursor_pos <= 7) {
			// Update mode when cursor over mode string
                        vfo.set_frequency(rotations, current_cursor_pos);
		    
                    } else if (current_cursor_pos == 8) {
			// Update mode when cursor under mode string
                        dsp.set_mode(rotations);
                        vfo.set_frequency(0, 0); // Set frequency on mode change - some modes have LO offsets
		    
                    } else {
			// Update filter width when cursor under filter width string
                        dsp.set_filter_width(rotations);
                    }
                }
            }
	    
	    // Update display and pass in FFT waterfall spectrum data
            display.update(magnitudes);
        } else {
            fft_ready = 0;
        }
    }
}
