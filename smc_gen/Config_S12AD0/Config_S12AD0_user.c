/*
* Copyright (c) 2016 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Config_S12AD0_user.c
* Component Version: 2.5.0
* Device(s)        : R5F52318AxFP
* Description      : This file implements device driver for Config_S12AD0.
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "Config_S12AD0.h"
/* Start user code for include. Do not edit comment generated here */
#include "dsp.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
// FFT sample buffers - defined in fft.cpp, filled here and processed in main loop
extern volatile int16_t i_fft[FFT_SIZE];
extern volatile int16_t q_fft[FFT_SIZE];
extern volatile uint8_t fft_ready;
static uint8_t fft_index = 0;

// Slow DC bias tracker for audio demodulation (time constant ~256 samples / ~5ms)
// Slow enough to preserve audio frequency content in the demodulated output
// measured_dc_i/q are extern'd in dsp.h and used by DSP::process()
static int32_t dc_bias_i = 3102 << 8;
static int32_t dc_bias_q = 3102 << 8;
volatile int16_t measured_dc_i = 3102;
volatile int16_t measured_dc_q = 3102;

// Fast DC bias tracker for FFT (time constant ~64 samples / ~1.3ms)
// Faster convergence prevents DC spike appearing at centre of waterfall display
static int32_t fft_dc_bias_i = 3102 << 6;
static int32_t fft_dc_bias_q = 3102 << 6;
static int16_t fft_dc_i = 3102;
static int16_t fft_dc_q = 3102;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_S12AD0_Create_UserInit
* Description  : This function adds user code after initializing the S12AD0 channel
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_S12AD0_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_S12AD0_interrupt
* Description  : This function is S12ADI interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#if FAST_INTERRUPT_VECTOR == VECT_S12AD_S12ADI0
#pragma interrupt r_Config_S12AD0_interrupt(vect=VECT(S12AD,S12ADI0),fint)
#else
#pragma interrupt r_Config_S12AD0_interrupt(vect=VECT(S12AD,S12ADI0))
#endif
static void r_Config_S12AD0_interrupt(void)
{
    /* Start user code for r_Config_S12AD0_interrupt. Do not edit comment generated here */
    // Read I and Q baseband samples from ADC result registers
    // AN000 = I channel (0x00089020), AN001 = Q channel (0x00089022)
    uint16_t i_raw = *((volatile uint16_t *)0x00089020U);
    uint16_t q_raw = *((volatile uint16_t *)0x00089022U);

    // Update slow DC tracker (used by audio demodulation path)
    // IIR: bias += (sample - bias/256), time constant ~256 samples
    dc_bias_i += ((int32_t)i_raw - (dc_bias_i >> 8));
    dc_bias_q += ((int32_t)q_raw - (dc_bias_q >> 8));
    measured_dc_i = (int16_t)(dc_bias_i >> 8);
    measured_dc_q = (int16_t)(dc_bias_q >> 8);

    // Update fast DC tracker (used by FFT path only)
    // IIR: bias += (sample - bias/64), time constant ~64 samples
    fft_dc_bias_i += ((int32_t)i_raw - (fft_dc_bias_i >> 6));
    fft_dc_bias_q += ((int32_t)q_raw - (fft_dc_bias_q >> 6));
    fft_dc_i = (int16_t)(fft_dc_bias_i >> 6);
    fft_dc_q = (int16_t)(fft_dc_bias_q >> 6);

    // Collect FFT samples using fast DC tracker
    // I scaled << 4 (x16), Q scaled << 5 (x32) to compensate for Q LO amplitude being
    // 0.5x I LO amplitude due to resistive quadrature divider (-6dB loss on Q channel)
    // int16_t range fills fix_fft input correctly for proper output scaling
    if (!fft_ready) {
        i_fft[fft_index] = (int16_t)(((int32_t)i_raw - fft_dc_i) << 4);
        q_fft[fft_index] = (int16_t)(((int32_t)q_raw - fft_dc_q) << 5);
        fft_index++;
        if (fft_index >= FFT_SIZE) {
            fft_index = 0;
            fft_ready = 1;
        }
    }

    // Demodulate/filter IQ samples and write audio output directly to DA1 register
    *((volatile uint16_t *)0x00088042U) = dsp_process(i_raw, q_raw);
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
