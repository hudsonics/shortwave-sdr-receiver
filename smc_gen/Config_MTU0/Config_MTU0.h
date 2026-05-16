/*
* Copyright (c) 2016 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Config_MTU0.h
* Component Version: 1.12.0
* Device(s)        : R5F52318AxFP
* Description      : This file implements device driver for Config_MTU0.
***********************************************************************************************************************/

#ifndef CFG_Config_MTU0_H
#define CFG_Config_MTU0_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_mtu2.h"

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define MTU0_PCLK_COUNTER_DIVISION      (1)
#define _03E7_TGRA0_VALUE               (0x03E7U) /* TGRA0 value */
#define _12BF_TGRB0_VALUE               (0x12BFU) /* TGRB0 value */
#define _12BF_TGRC0_VALUE               (0x12BFU) /* TGRC0 value */
#define _12BF_TGRD0_VALUE               (0x12BFU) /* TGRD0 value */
#define _12BF_TGRE0_VALUE               (0x12BFU) /* TGRE0 value */
#define _12BF_TGRF0_VALUE               (0x12BFU) /* TGRF0 value */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_MTU0_Create(void);
void R_Config_MTU0_Create_UserInit(void);
void R_Config_MTU0_Start(void);
void R_Config_MTU0_Stop(void);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
