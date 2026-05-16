/*
* u8g2_rx231.h
* ============
*
* Header for u8g2 library hardware abstraction layer (HAL)
* - Gives the library functions for I2C communication using RIIC0 and builtin software delays
*/

#ifndef U8G2_RX231_H
#define U8G2_RX231_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u8g2.h"

uint8_t u8x8_byte_rx231_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay_rx231(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#ifdef __cplusplus
}
#endif

#endif
