/*
 * si5351_rx231.cpp
 * ================
 *
 * RX231 I2C hardware abstraction layer (HAL) for Si5351 clock generator IC
 * - Implements si5351_write, si5351_write_bulk and si5351_read as Si5351 class methods, replacing the original Arduino Wire.h based implementation
 * - Uses RIIC0 master send/receive with timeout to prevent lockup on I2C errors
 * - I2C completion is signalled via g_i2c_tx_done/g_i2c_rx_done flags set in Config_RIIC0_user.c interrupt callbacks
 *
 * si5351 library available at: https://github.com/etherkit/Si5351Arduino
 */

#include "si5351.h"
#include "si5351_rx231.h"

extern "C" {
    #include "r_smc_entry.h"
}

uint8_t Si5351::si5351_write(uint8_t addr, uint8_t data)
{
    uint8_t tx[2];
    tx[0] = addr;
    tx[1] = data;
    g_i2c_tx_done = 0;
    g_i2c_error   = 0;
    uint32_t timeout = I2C_TIMEOUT_US;
    while (R_Config_RIIC0_Master_Send(SI5351_BUS_BASE_ADDR, tx, 2) != MD_OK)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
        if (--timeout == 0) return 1;
    }
    WAIT_FOR_FLAG(g_i2c_tx_done);
    return g_i2c_error;
}

uint8_t Si5351::si5351_write_bulk(uint8_t addr, uint8_t bytes, uint8_t *data)
{
    uint8_t tx[16];
    tx[0] = addr;
    for (int i = 0; i < bytes; i++) {
        tx[i + 1] = data[i];
    }
    g_i2c_tx_done = 0;
    g_i2c_error   = 0;
    uint32_t timeout = I2C_TIMEOUT_US;
    while (R_Config_RIIC0_Master_Send(SI5351_BUS_BASE_ADDR, tx, bytes + 1) != MD_OK)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
        if (--timeout == 0) return 1;
    }
    WAIT_FOR_FLAG(g_i2c_tx_done);
    return g_i2c_error;
}

uint8_t Si5351::si5351_read(uint8_t addr)
{
    uint8_t tx[1] = { addr };
    uint8_t rx[1] = { 0x00 };
    uint32_t timeout;

    g_i2c_tx_done = 0;
    g_i2c_error   = 0;
    timeout = I2C_TIMEOUT_US;
    while (R_Config_RIIC0_Master_Send(SI5351_BUS_BASE_ADDR, tx, 1) != MD_OK) {
        R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
        if (--timeout == 0) return 0x00;
    }
    WAIT_FOR_FLAG(g_i2c_tx_done);
    if (g_i2c_error) return 0x00;

    g_i2c_rx_done = 0;
    g_i2c_error   = 0;
    timeout = I2C_TIMEOUT_US;
    while (R_Config_RIIC0_Master_Receive(SI5351_BUS_BASE_ADDR, rx, 1) != MD_OK)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_MICROSECS);
        if (--timeout == 0) return 0x00;
    }
    WAIT_FOR_FLAG(g_i2c_rx_done);
    if (g_i2c_error) return 0x00;
    return rx[0];
}