/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "nrf52.h"
#include "nrf_delay.h"

/*
#include "LL_Hardware_cfg.h"    // LL_TYPE_OF_BUZZER
#include "LL_LED_Helmet.h"      // should be LL_Buzzer.h
#include "LL_Reset.h"

#define LL_BUZZER_BEEP_SHORT     100 // ms
#define LL_BUZZER_BEEP_LONG     1000 // ms

static unsigned long sgulResetReasons = 0;
*/



unsigned long LL_Reset_Reasons(void)
{
    unsigned long reason = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = 0xFFFFFFFF; // A field is cleared by writing '1' to it. (see RESETREAS in RM.)
    return reason;
}

/*
static void LL_Reset_Reasons_BuzzerBeep(unsigned long ms)
{
    if(0 == LL_TYPE_OF_BUZZER) { 
                // do 3KHz by software:
                for(int i = 0; i < (ms*1000/666); i++) {
                    LL_LED_ON( E_LL_PWM_BUZZER); nrf_delay_us(333);
                    LL_LED_OFF(E_LL_PWM_BUZZER); nrf_delay_us(333);
                }
    } else {
                // do ?KHz by buzzer hardware:
                LL_LED_ON( E_LL_PWM_BUZZER); nrf_delay_ms(ms);
    }
    // off the buzzer:
    LL_LED_OFF(E_LL_PWM_BUZZER);
    nrf_delay_ms(1000);
}
void LL_Reset_ReasonsIndicateByBeep(void)
{
//  unsigned long ulResetReasons = NRF_POWER->RESETREAS; NRF_POWER->RESETREAS = 0xFFFFFFFF;

//  if(1 != LL_FUNC_RESET_REASONS_INDICATE_BY_BEEP) return;

    switch(sgulResetReasons) {
        case (1<<0): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            break;
        case (1<<1): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            break;
        case (1<<2): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            break;
        case (1<<3): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            break;
        case (1<<16): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            break;
        case (1<<17): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            break;
        case (1<<18): 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_SHORT);
            break;
        default: 
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            LL_Reset_Reasons_BuzzerBeep(LL_BUZZER_BEEP_LONG);
            break;
    }
}
*/
