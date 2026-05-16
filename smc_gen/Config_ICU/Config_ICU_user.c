/*
* Copyright (c) 2016 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Config_ICU_user.c
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
extern void RotaryEncoder_ISR_Handler(void);
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_ICU_Create_UserInit
* Description  : This function adds user code after initializing the ICU module
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_ICU_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_ICU_software_interrupt
* Description  : This function is SWINT interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#if FAST_INTERRUPT_VECTOR == VECT_ICU_SWINT
#pragma interrupt r_Config_ICU_software_interrupt(vect=VECT(ICU,SWINT),fint)
#else
#pragma interrupt r_Config_ICU_software_interrupt(vect=VECT(ICU,SWINT))
#endif
static void r_Config_ICU_software_interrupt(void)
{
    /* Start user code for r_Config_ICU_software_interrupt. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_ICU_irq1_interrupt
* Description  : This function is IRQ1 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#if FAST_INTERRUPT_VECTOR == VECT_ICU_IRQ1
#pragma interrupt r_Config_ICU_irq1_interrupt(vect=VECT(ICU,IRQ1),fint)
#else
#pragma interrupt r_Config_ICU_irq1_interrupt(vect=VECT(ICU,IRQ1))
#endif
static void r_Config_ICU_irq1_interrupt(void)
{
    /* Start user code for r_Config_ICU_irq1_interrupt. Do not edit comment generated here */
    RotaryEncoder_ISR_Handler();
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_ICU_irq2_interrupt
* Description  : This function is IRQ2 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#if FAST_INTERRUPT_VECTOR == VECT_ICU_IRQ2
#pragma interrupt r_Config_ICU_irq2_interrupt(vect=VECT(ICU,IRQ2),fint)
#else
#pragma interrupt r_Config_ICU_irq2_interrupt(vect=VECT(ICU,IRQ2))
#endif
static void r_Config_ICU_irq2_interrupt(void)
{
    /* Start user code for r_Config_ICU_irq2_interrupt. Do not edit comment generated here */
    RotaryEncoder_ISR_Handler();
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

