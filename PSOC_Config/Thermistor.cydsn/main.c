/*******************************************************************************
* File Name: main.c
*
* Version: 1.20
*
* Description:
*  This is source code for the datasheet example of the Thermistor Calculator
*  component.
*
********************************************************************************
* Copyright 2012-2014, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>
#include <stdio.h> 

/* Defines for AMux channels */
#define THERM_REF_CHANNEL       (0u)
#define THERM_SIGNAL_CHANNEL    (1u)

#define USBFS_DEVICE    (0u)


/* More samples means smoother data but values may lag
   powers of 2 make the math easier for the CPU and should be used. */
#define SAMPLES                 (256u)

/* Degree sign */ 
#define DEGREE                  (0xDFu)


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  The main function of the Thermistor Calculator datasheet example project. 
*  For a detailed description please refer to the project schematic.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
    int32 temperature = 0;
    int32 absTemp;
    int16 vRef, vTherm;
    char str[16];

CY_ISR(function){
        sprintf(str,"%ld.%02ldF\r\n",absTemp/100,absTemp%100);
        UART_1_PutString(str);
        timer_isr_ClearPending();
}

int main()
{

    /* used to denote the first sampling time */
    uint8 init = 0u;
    
    /* Start ADC, AMux, OpAmp components */
	ADC_DelSig_Start();
	AMux_Start();
	Opamp_Start();
    UART_1_Start();
    Timer_1_Start();
    timer_isr_StartEx(function);
 //   timer_isr_Start();
    
    CyGlobalIntEnable;
    
    for(;;)
    {
        
        /* Read voltage across thermistor */
        AMux_FastSelect(THERM_SIGNAL_CHANNEL);
        ADC_DelSig_StartConvert();
        ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT);
        vTherm = ADC_DelSig_GetResult32();
    	
        /* Read voltage across reference resistor */
        AMux_FastSelect(THERM_REF_CHANNEL);
        ADC_DelSig_StartConvert();
        ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT);
        vRef = ADC_DelSig_GetResult32();
    
        /* Find temperature in Celsius degree using the component API */
        temperature += Thermistor_GetTemperature(Thermistor_GetResistance(vRef, vTherm));

        /* If we are initialized */
        if(init)
        {
            /* Averaging over a number of samples, gives a much smoother look to the numbers */
            temperature -= (temperature/SAMPLES);
        }else{
            /* First time through the loop, no data, so replicate the first value */
            temperature *= SAMPLES;
        }
        init = 1;
       
        
        
        absTemp = temperature;
        absTemp /= SAMPLES;
        
        //sprintf(str,"%ld.%02ldC\r\n",absTemp/100,absTemp%100);
        //UART_1_PutString(str);
        
        
      //    CyDelay(500);
    }
  
}


/* [] END OF FILE */
