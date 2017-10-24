/*******************************************************************************
* File Name: V_hi.h  
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

#if !defined(CY_PINS_V_hi_H) /* Pins V_hi_H */
#define CY_PINS_V_hi_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "V_hi_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    V_hi_Write(uint8 value) ;
void    V_hi_SetDriveMode(uint8 mode) ;
uint8   V_hi_ReadDataReg(void) ;
uint8   V_hi_Read(void) ;
void    V_hi_SetInterruptMode(uint16 position, uint16 mode) ;
uint8   V_hi_ClearInterrupt(void) ;
/** @} general */

/***************************************
*           API Constants        
***************************************/

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the V_hi_SetDriveMode() function.
     *  @{
     */
        /* Drive Modes */
        #define V_hi_DM_ALG_HIZ         PIN_DM_ALG_HIZ   /**< \brief High Impedance Analog   */
        #define V_hi_DM_DIG_HIZ         PIN_DM_DIG_HIZ   /**< \brief High Impedance Digital  */
        #define V_hi_DM_RES_UP          PIN_DM_RES_UP    /**< \brief Resistive Pull Up       */
        #define V_hi_DM_RES_DWN         PIN_DM_RES_DWN   /**< \brief Resistive Pull Down     */
        #define V_hi_DM_OD_LO           PIN_DM_OD_LO     /**< \brief Open Drain, Drives Low  */
        #define V_hi_DM_OD_HI           PIN_DM_OD_HI     /**< \brief Open Drain, Drives High */
        #define V_hi_DM_STRONG          PIN_DM_STRONG    /**< \brief Strong Drive            */
        #define V_hi_DM_RES_UPDWN       PIN_DM_RES_UPDWN /**< \brief Resistive Pull Up/Down  */
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define V_hi_MASK               V_hi__MASK
#define V_hi_SHIFT              V_hi__SHIFT
#define V_hi_WIDTH              1u

/* Interrupt constants */
#if defined(V_hi__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in V_hi_SetInterruptMode() function.
     *  @{
     */
        #define V_hi_INTR_NONE      (uint16)(0x0000u)   /**< \brief Disabled             */
        #define V_hi_INTR_RISING    (uint16)(0x0001u)   /**< \brief Rising edge trigger  */
        #define V_hi_INTR_FALLING   (uint16)(0x0002u)   /**< \brief Falling edge trigger */
        #define V_hi_INTR_BOTH      (uint16)(0x0003u)   /**< \brief Both edge trigger    */
        /** @} intrMode */
/** @} group_constants */

    #define V_hi_INTR_MASK      (0x01u)
#endif /* (V_hi__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define V_hi_PS                     (* (reg8 *) V_hi__PS)
/* Data Register */
#define V_hi_DR                     (* (reg8 *) V_hi__DR)
/* Port Number */
#define V_hi_PRT_NUM                (* (reg8 *) V_hi__PRT) 
/* Connect to Analog Globals */                                                  
#define V_hi_AG                     (* (reg8 *) V_hi__AG)                       
/* Analog MUX bux enable */
#define V_hi_AMUX                   (* (reg8 *) V_hi__AMUX) 
/* Bidirectional Enable */                                                        
#define V_hi_BIE                    (* (reg8 *) V_hi__BIE)
/* Bit-mask for Aliased Register Access */
#define V_hi_BIT_MASK               (* (reg8 *) V_hi__BIT_MASK)
/* Bypass Enable */
#define V_hi_BYP                    (* (reg8 *) V_hi__BYP)
/* Port wide control signals */                                                   
#define V_hi_CTL                    (* (reg8 *) V_hi__CTL)
/* Drive Modes */
#define V_hi_DM0                    (* (reg8 *) V_hi__DM0) 
#define V_hi_DM1                    (* (reg8 *) V_hi__DM1)
#define V_hi_DM2                    (* (reg8 *) V_hi__DM2) 
/* Input Buffer Disable Override */
#define V_hi_INP_DIS                (* (reg8 *) V_hi__INP_DIS)
/* LCD Common or Segment Drive */
#define V_hi_LCD_COM_SEG            (* (reg8 *) V_hi__LCD_COM_SEG)
/* Enable Segment LCD */
#define V_hi_LCD_EN                 (* (reg8 *) V_hi__LCD_EN)
/* Slew Rate Control */
#define V_hi_SLW                    (* (reg8 *) V_hi__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define V_hi_PRTDSI__CAPS_SEL       (* (reg8 *) V_hi__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define V_hi_PRTDSI__DBL_SYNC_IN    (* (reg8 *) V_hi__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define V_hi_PRTDSI__OE_SEL0        (* (reg8 *) V_hi__PRTDSI__OE_SEL0) 
#define V_hi_PRTDSI__OE_SEL1        (* (reg8 *) V_hi__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define V_hi_PRTDSI__OUT_SEL0       (* (reg8 *) V_hi__PRTDSI__OUT_SEL0) 
#define V_hi_PRTDSI__OUT_SEL1       (* (reg8 *) V_hi__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define V_hi_PRTDSI__SYNC_OUT       (* (reg8 *) V_hi__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(V_hi__SIO_CFG)
    #define V_hi_SIO_HYST_EN        (* (reg8 *) V_hi__SIO_HYST_EN)
    #define V_hi_SIO_REG_HIFREQ     (* (reg8 *) V_hi__SIO_REG_HIFREQ)
    #define V_hi_SIO_CFG            (* (reg8 *) V_hi__SIO_CFG)
    #define V_hi_SIO_DIFF           (* (reg8 *) V_hi__SIO_DIFF)
#endif /* (V_hi__SIO_CFG) */

/* Interrupt Registers */
#if defined(V_hi__INTSTAT)
    #define V_hi_INTSTAT             (* (reg8 *) V_hi__INTSTAT)
    #define V_hi_SNAP                (* (reg8 *) V_hi__SNAP)
    
	#define V_hi_0_INTTYPE_REG 		(* (reg8 *) V_hi__0__INTTYPE)
#endif /* (V_hi__INTSTAT) */

#endif /* End Pins V_hi_H */


/* [] END OF FILE */
