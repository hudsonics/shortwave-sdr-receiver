/*
* u8g2_rx231.c
* ============
*
* Hardware abstraction layer (HAL) for u8g2 library
* - Interface for u8g2 to communicate over I2C using RIIC0
* - Call software delay functions
* - Interface with display reset pin controlled by MCU - not needed in this 
*
* u8g2 library available at: https://github.com/olikraus/u8g2
*/

#include "u8g2.h"
#include "r_smc_entry.h"

// I2C status flags RIIC0 config
extern volatile uint8_t g_i2c_tx_done;
extern volatile uint8_t g_i2c_rx_done;
extern volatile uint8_t g_i2c_error;

#define I2C_TIMEOUT_US  50000UL

// Byte communication callback - called by u8g2 to send I2C data
uint8_t u8x8_byte_rx231_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    // u8g2 sends max 32 bytes per transaction (1 command + 31 data) - buffer set bigger just in case..
    // TODO: set buffer size properly and check for glitches
    static uint8_t buffer[40]; 
    
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg) {
    case U8X8_MSG_BYTE_INIT:
        // Nothing needed - RIIC0 already initialised
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        // Reset buffer at start of each transaction
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_SEND:
        // Accumulate bytes into buffer
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0) {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER: {
        g_i2c_tx_done	= 0;
        g_i2c_error		= 0;

        // Check transfer complete - wait by specified timeout in case race condition
	// TODO: tune timeout period.
        uint32_t timeout = I2C_TIMEOUT_US;

	// Wait for RIIC0 to accept transfer request
        while (R_Config_RIIC0_Master_Send(u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx) != MD_OK) {
            R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
            if (--timeout == 0) {
                return(0);
            }
        }

        timeout = I2C_TIMEOUT_US;

	// Wait for transfer complete interrupt flag
        while (!g_i2c_tx_done && !g_i2c_error) {
            R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
            if (--timeout == 0) {
                return(0);
            }
        }

        if (g_i2c_error) {
            return(0);
        }

        // Transaction success!
        return(1);
    }

    default:
        return(0);
    }

    return(1);
}

// GPIO and delay callback - called by u8g2 for delays and pins
uint8_t u8x8_gpio_and_delay_rx231(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        // Nothing needed - already initialised by default
        break;

    // Builtin delay functions.
    case U8X8_MSG_DELAY_MILLI:
        R_BSP_SoftwareDelay(arg_int, BSP_DELAY_MILLISECS);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        R_BSP_SoftwareDelay(arg_int * 10, BSP_DELAY_MICROSECS);
        break;

    case U8X8_MSG_DELAY_100NANO:
        // 1us delay is shortest delay available
        R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
        break;

    case U8X8_MSG_GPIO_RESET:
        // OLED display has no reset pin
        break;

    default:
        return(0);
    }

    return(1);
}
