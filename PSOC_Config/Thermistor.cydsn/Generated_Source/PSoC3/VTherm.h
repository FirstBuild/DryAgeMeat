/*******************************************************************************
* File Name: VTherm.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_VTherm_H) /* Pins VTherm_H */
#define CY_PINS_VTherm_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "VTherm_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    VTherm_Write(uint8 value) ;
void    VTherm_SetDriveMode(uint8 mode) ;
uint8   VTherm_ReadDataReg(void) ;
uint8   VTherm_Read(void) ;
void    VTherm_SetInterruptMode(uint16 position, uint16 mode) ;
uint8   VTherm_ClearInterrupt(void) ;
/** @} general */

/***************************************
*           API Constants        
***************************************/

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the VTherm_SetDriveMode() function.
     *  @{
     */
        /* Drive Modes */
        #define VTherm_DM_ALG_HIZ         PIN_DM_ALG_HIZ   /**< \brief High Impedance Analog   */
        #define VTherm_DM_DIG_HIZ         PIN_DM_DIG_HIZ   /**< \brief High Impedance Digital  */
        #define VTherm_DM_RES_UP          PIN_DM_RES_UP    /**< \brief Resistive Pull Up       */
        #define VTherm_DM_RES_DWN         PIN_DM_RES_DWN   /**< \brief Resistive Pull Down     */
        #define VTherm_DM_OD_LO           PIN_DM_OD_LO     /**< \brief Open Drain, Drives Low  */
        #define VTherm_DM_OD_HI           PIN_DM_OD_HI     /**< \brief Open Drain, Drives High */
        #define VTherm_DM_STRONG          PIN_DM_STRONG    /**< \brief Strong Drive            */
        #define VTherm_DM_RES_UPDWN       PIN_DM_RES_UPDWN /**< \brief Resistive Pull Up/Down  */
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define VTherm_MASK               VTherm__MASK
#define VTherm_SHIFT              VTherm__SHIFT
#define VTherm_WIDTH              1u

/* Interrupt constants */
#if defined(VTherm__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in VTherm_SetInterruptMode() function.
     *  @{
     */
        #define VTherm_INTR_NONE      (uint16)(0x0000u)   /**< \brief Disabled             */
        #define VTherm_INTR_RISING    (uint16)(0x0001u)   /**< \brief Rising edge trigger  */
        #define VTherm_INTR_FALLING   (uint16)(0x0002u)   /**< \brief Falling edge trigger */
        #define VTherm_INTR_BOTH      (uint16)(0x0003u)   /**< \brief Both edge trigger    */
        /** @} intrMode */
/** @} group_constants */

    #define VTherm_INTR_MASK      (0x01u)
#endif /* (VTherm__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define VTherm_PS                     (* (reg8 *) VTherm__PS)
/* Data Register */
#define VTherm_DR                     (* (reg8 *) VTherm__DR)
/* Port Number */
#define VTherm_PRT_NUM                (* (reg8 *) VTherm__PRT) 
/* Connect to Analog Globals */                                                  
#define VTherm_AG                     (* (reg8 *) VTherm__AG)                       
/* Analog MUX bux enable */
#define VTherm_AMUX                   (* (reg8 *) VTherm__AMUX) 
/* Bidirectional Enable */                                                        
#define VTherm_BIE                    (* (reg8 *) VTherm__BIE)
/* Bit-mask for Aliased Register Access */
#define VTherm_BIT_MASK               (* (reg8 *) VTherm__BIT_MASK)
/* Bypass Enable */
#define VTherm_BYP                    (* (reg8 *) VTherm__BYP)
/* Port wide control signals */                                                   
#define VTherm_CTL                    (* (reg8 *) VTherm__CTL)
/* Drive Modes */
#define VTherm_DM0                    (* (reg8 *) VTherm__DM0) 
#define VTherm_DM1                    (* (reg8 *) VTherm__DM1)
#define VTherm_DM2                    (* (reg8 *) VTherm__DM2) 
/* Input Buffer Disable Override */
#define VTherm_INP_DIS                (* (reg8 *) VTherm__INP_DIS)
/* LCD Common or Segment Drive */
#define VTherm_LCD_COM_SEG            (* (reg8 *) VTherm__LCD_COM_SEG)
/* Enable Segment LCD */
#define VTherm_LCD_EN                 (* (reg8 *) VTherm__LCD_EN)
/* Slew Rate Control */
#define VTherm_SLW                    (* (reg8 *) VTherm__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define VTherm_PRTDSI__CAPS_SEL       (* (reg8 *) VTherm__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define VTherm_PRTDSI__DBL_SYNC_IN    (* (reg8 *) VTherm__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define VTherm_PRTDSI__OE_SEL0        (* (reg8 *) VTherm__PRTDSI__OE_SEL0) 
#define VTherm_PRTDSI__OE_SEL1        (* (reg8 *) VTherm__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define VTherm_PRTDSI__OUT_SEL0       (* (reg8 *) VTherm__PRTDSI__OUT_SEL0) 
#define VTherm_PRTDSI__OUT_SEL1       (* (reg8 *) VTherm__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define VTherm_PRTDSI__SYNC_OUT       (* (reg8 *) VTherm__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(VTherm__SIO_CFG)
    #define VTherm_SIO_HYST_EN        (* (reg8 *) VTherm__SIO_HYST_EN)
    #define VTherm_SIO_REG_HIFREQ     (* (reg8 *) VTherm__SIO_REG_HIFREQ)
    #define VTherm_SIO_CFG            (* (reg8 *) VTherm__SIO_CFG)
    #define VTherm_SIO_DIFF           (* (reg8 *) VTherm__SIO_DIFF)
#endif /* (VTherm__SIO_CFG) */

/* Interrupt Registers */
#if defined(VTherm__INTSTAT)
    #define VTherm_INTSTAT             (* (reg8 *) VTherm__INTSTAT)
    #define VTherm_SNAP                (* (reg8 *) VTherm__SNAP)
    
	#define VTherm_0_INTTYPE_REG 		(* (reg8 *) VTherm__0__INTTYPE)
#endif /* (VTherm__INTSTAT) */

#endif /* End Pins VTherm_H */


/* [] END OF FILE */
