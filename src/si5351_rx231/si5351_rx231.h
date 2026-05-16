/*
* si5351_rx231.h
* ============
*
* Header for si5351 library hardware abstraction layer (HAL)
* - Gives the library functions for I2C communication using RIIC0 and timeout macro function using builtin software delays
*/

#ifndef si5351_rx231_h
#define si5351_rx231_h

#include <stdint.h>

extern "C" {
	#include "r_smc_entry.h"
}

extern "C" {
	extern volatile uint8_t g_i2c_tx_done;
	extern volatile uint8_t g_i2c_rx_done;
	extern volatile uint8_t g_i2c_error;
}

#define I2C_TIMEOUT_US      50000UL
#define WAIT_FOR_FLAG(f)    { uint32_t t = I2C_TIMEOUT_US; while (!(f) && --t) R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS); }

#ifdef __cplusplus
extern "C" {
#endif
extern volatile uint8_t g_i2c_tx_done;
extern volatile uint8_t g_i2c_rx_done;
extern volatile uint8_t g_i2c_error;
#ifdef __cplusplus
}
#endif

#endif // si5351_rx231_h