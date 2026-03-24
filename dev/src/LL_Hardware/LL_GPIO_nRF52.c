/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "LL_GPIO.h"



void LL_GPIO_InputCfg(
                    unsigned long ulPort    ,
                    unsigned long ulPin     ,
                    unsigned long ulPull    ,
                    unsigned long ulTrigger )
{
    nrf_gpio_pin_pull_t nrfPull = NRF_GPIO_PIN_NOPULL;    
    switch(ulPull) {
        case LL_GPIO_PULL_UP  : nrfPull = NRF_GPIO_PIN_PULLUP;   break;
        case LL_GPIO_PULL_DOWN: nrfPull = NRF_GPIO_PIN_PULLDOWN; break;
    }
      
    if (        LL_GPIO_TRIGGER_HIGH == ulTrigger ) { nrf_gpio_cfg_sense_input( ulPin, nrfPull, NRF_GPIO_PIN_SENSE_HIGH ); nrf_delay_ms(1); // Workaround for PAN_028 rev1.1 anomaly 22 - System: Issues with disable System OFF mechanism
    } else if ( LL_GPIO_TRIGGER_LOW  == ulTrigger ) { nrf_gpio_cfg_sense_input( ulPin, nrfPull, NRF_GPIO_PIN_SENSE_LOW  ); nrf_delay_ms(1); // Workaround for PAN_028 rev1.1 anomaly 22 - System: Issues with disable System OFF mechanism
    } else                                          { nrf_gpio_cfg_input(       ulPin, nrfPull                          );   
    }
}

unsigned long LL_GPIO_InputRead(
                    unsigned long ulPort    ,
                    unsigned long ulPin     )
{
    return (((NRF_GPIO->IN)&(1<<ulPin))>>ulPin);
}

void LL_GPIO_OutputCfg(
                    unsigned long ulPort ,
                    unsigned long ulPin  ,
                    unsigned long ulPull ,
                    unsigned long init   )
{
    NRF_GPIO->DIRSET = 1<<ulPin;
    
    // pull, see RM PIN_CNF[n].
    unsigned long ulPullVal = 0;
    switch(ulPull) {
        case LL_GPIO_PULL_UP  : ulPullVal = 3; break;
        case LL_GPIO_PULL_DOWN: ulPullVal = 1; break;
    }
    NRF_GPIO->PIN_CNF[ulPin] &=     ~(0x3 << 2) ;
    NRF_GPIO->PIN_CNF[ulPin] |= ulPullVal << 2  ;    
    
    // init
    if(1 == init) { NRF_GPIO->OUTSET = 1 << ulPin; }
    else          { NRF_GPIO->OUTCLR = 1 << ulPin; }
}

void LL_GPIO_OutputWrite(
                    unsigned long ulPort ,
                    unsigned long ulPin  ,
                    unsigned long value  )
{
    if(1 == value) { NRF_GPIO->OUTSET = 1 << ulPin; }
    else           { NRF_GPIO->OUTCLR = 1 << ulPin; }
}
