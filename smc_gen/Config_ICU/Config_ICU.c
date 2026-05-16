/*
* Copyright (c) 2016 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Config_ICU.c
* Component Version: 2.3.0
* Device(s)        : R5F52318AxFP
* Description      : This file implements device driver for Config_ICU.
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
#include "Config_ICU.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_ICU_Create
* Description  : This function initializes the ICU module
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_Create(void)
{
    /* Disable IRQ interrupts */
    ICU.IER[0x08].BYTE = 0x00U;
    /* Disable software interrupt */
    IEN(ICU,SWINT) = 0U;

    /* Disable IRQ digital filter */
    ICU.IRQFLTE0.BYTE &= ~(_02_ICU_IRQ1_FILTER_ENABLE | _04_ICU_IRQ2_FILTER_ENABLE);

    /* Set IRQ1 pin */
    MPC.P31PFS.BYTE = 0x40U;
    PORT3.PDR.BYTE &= 0xFDU;
    PORT3.PMR.BYTE &= 0xFDU;

    /* Set IRQ2 pin */
    MPC.P32PFS.BYTE = 0x40U;
    PORT3.PDR.BYTE &= 0xFBU;
    PORT3.PMR.BYTE &= 0xFBU;

    /* Set IRQ detection type */
    ICU.IRQCR[1].BYTE = _0C_ICU_IRQ_EDGE_BOTH;
    IR(ICU,IRQ1) = 0U;
    ICU.IRQCR[2].BYTE = _0C_ICU_IRQ_EDGE_BOTH;
    IR(ICU,IRQ2) = 0U;

    /* Set SWINT priority level */
    IPR(ICU,SWINT) = _0F_ICU_PRIORITY_LEVEL15;

    /* Set IRQ priority level */
    IPR(ICU,IRQ1) = _01_ICU_PRIORITY_LEVEL1;
    IPR(ICU,IRQ2) = _01_ICU_PRIORITY_LEVEL1;

    R_Config_ICU_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_Software_Start
* Description  : This function enables SWINT interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_Software_Start(void)
{
    /* Enable software interrupt */
    IEN(ICU,SWINT) = 1U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_SoftwareInterrupt_Generate
* Description  : This function generates SWINT interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_SoftwareInterrupt_Generate(void)
{
    /* Generate software interrupt */
    ICU.SWINTR.BIT.SWINT = 1U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_Software_Stop
* Description  : This function disables SWINT interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_Software_Stop(void)
{
    /* Disable software interrupt */
    IEN(ICU,SWINT) = 0U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_IRQ1_Start
* Description  : This function enables IRQ1 interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_IRQ1_Start(void)
{
    /* Enable IRQ1 interrupt */
    IEN(ICU,IRQ1) = 1U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_IRQ1_Stop
* Description  : This function disables IRQ1 interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_IRQ1_Stop(void)
{
    /* Disable IRQ1 interrupt */
    IEN(ICU,IRQ1) = 0U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_IRQ2_Start
* Description  : This function enables IRQ2 interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_IRQ2_Start(void)
{
    /* Enable IRQ2 interrupt */
    IEN(ICU,IRQ2) = 1U;
}

/***********************************************************************************************************************
* Function Name: R_Config_ICU_IRQ2_Stop
* Description  : This function disables IRQ2 interrupt
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_IRQ2_Stop(void)
{
    /* Disable IRQ2 interrupt */
    IEN(ICU,IRQ2) = 0U;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
