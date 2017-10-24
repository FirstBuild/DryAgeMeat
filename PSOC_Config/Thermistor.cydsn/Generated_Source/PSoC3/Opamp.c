/*******************************************************************************
* File Name: Opamp.c
* Version 1.90
*
* Description:
*  This file provides the source code to the API for OpAmp (Analog Buffer) 
*  Component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "Opamp.h"

uint8 Opamp_initVar = 0u;


/*******************************************************************************   
* Function Name: Opamp_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the 
*  customizer of the component placed onto schematic. Usually called in 
*  Opamp_Start().
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Opamp_Init(void) 
{
    Opamp_SetPower(Opamp_DEFAULT_POWER);
}


/*******************************************************************************   
* Function Name: Opamp_Enable
********************************************************************************
*
* Summary:
*  Enables the OpAmp block operation
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Opamp_Enable(void) 
{
    /* Enable negative charge pumps in ANIF */
    Opamp_PUMP_CR1_REG  |= (Opamp_PUMP_CR1_CLKSEL | Opamp_PUMP_CR1_FORCE);

    /* Enable power to buffer in active mode */
    Opamp_PM_ACT_CFG_REG |= Opamp_ACT_PWR_EN;

    /* Enable power to buffer in alternative active mode */
    Opamp_PM_STBY_CFG_REG |= Opamp_STBY_PWR_EN;
}


/*******************************************************************************
* Function Name:   Opamp_Start
********************************************************************************
*
* Summary:
*  The start function initializes the Analog Buffer with the default values and 
*  sets the power to the given level. A power level of 0, is same as 
*  executing the stop function.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Opamp_initVar: Used to check the initial configuration, modified 
*  when this function is called for the first time.
*
*******************************************************************************/
void Opamp_Start(void) 
{
    if(Opamp_initVar == 0u)
    {
        Opamp_initVar = 1u;
        Opamp_Init();
    }

    Opamp_Enable();
}


/*******************************************************************************
* Function Name: Opamp_Stop
********************************************************************************
*
* Summary:
*  Powers down amplifier to lowest power state.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Opamp_Stop(void) 
{
    /* Disable power to buffer in active mode template */
    Opamp_PM_ACT_CFG_REG &= (uint8)(~Opamp_ACT_PWR_EN);

    /* Disable power to buffer in alternative active mode template */
    Opamp_PM_STBY_CFG_REG &= (uint8)(~Opamp_STBY_PWR_EN);
    
    /* Disable negative charge pumps for ANIF only if all ABuf is turned OFF */
    if(Opamp_PM_ACT_CFG_REG == 0u)
    {
        Opamp_PUMP_CR1_REG &= (uint8)(~(Opamp_PUMP_CR1_CLKSEL | Opamp_PUMP_CR1_FORCE));
    }
}


/*******************************************************************************
* Function Name: Opamp_SetPower
********************************************************************************
*
* Summary:
*  Sets power level of Analog buffer.
*
* Parameters: 
*  power: PSoC3: Sets power level between low (1) and high power (3).
*         PSoC5: Sets power level High (0)
*
* Return:
*  void
*
**********************************************************************************/
void Opamp_SetPower(uint8 power) 
{
    #if (CY_PSOC3 || CY_PSOC5LP)
        Opamp_CR_REG &= (uint8)(~Opamp_PWR_MASK);
        Opamp_CR_REG |= power & Opamp_PWR_MASK;      /* Set device power */
    #else
        CYASSERT(Opamp_HIGHPOWER == power);
    #endif /* CY_PSOC3 || CY_PSOC5LP */
}


/* [] END OF FILE */
