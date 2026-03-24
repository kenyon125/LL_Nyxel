/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "SEGGER_RTT.h"

#include "LL_Hardware_cfg.h"
#include "LL_Power.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_PWM.h"
#include "LL_LED_Helmet.h"
#include "LL_ADC.h"
#include "LL_Flash.h"
#include "esb_timeslot.h"
#include "LL_Para.h"
#include "LL_Battery.h"
#include "LL_BatteryIndicator.h"


typedef enum {
    E_STEP__BATTERY_INDICATOR__OFF = 0,
    // indicator when power on
    E_STEP__BATTERY_INDICATOR__PWR_ON__CHK_BATT,    
        // case battery enough
        E_STEP__BATTERY_INDICATOR__PWR_ON__START_UP,   
        E_STEP__BATTERY_INDICATOR__PWR_ON__FLASH_BATT,   
        // case battery need charge
        E_STEP__BATTERY_INDICATOR__PWR_ON__ALERT_BATT,  
    // indicator when runtime
    E_STEP__BATTERY_INDICATOR__RUNTIME__CHK_BATT,   
        // case battery need charge
        E_STEP__BATTERY_INDICATOR__RUNTIME__ALERT_BATT, 
} E_STEP__BATTERY_INDICATOR;
static E_STEP__BATTERY_INDICATOR step= E_STEP__BATTERY_INDICATOR__OFF;

static unsigned long delay_cnt = 0;
static unsigned long rep_cnt   = 0;



void LL_BatteryIndicator__Start(void) {
    step = E_STEP__BATTERY_INDICATOR__PWR_ON__CHK_BATT;
}


void LL_BatteryIndicator__Stop(void) {
    step = E_STEP__BATTERY_INDICATOR__OFF;
}


void LL_BatteryIndicator__ShowInPanel(unsigned long ulBatteryLevel) {
    if(ulBatteryLevel < 30) { 
        //LL_LED_Panel_SingleFrameBufInit(); LL_LED_Panel_SingleFrameBufSetNumber(ulBatteryLevel, LL_LED_Panel_GetColorIdxOfRED()); LL_LED_Panel_SingleFrameBufDisplay();
    } else if(ulBatteryLevel < 70) { 
        //LL_LED_Panel_SingleFrameBufInit(); LL_LED_Panel_SingleFrameBufSetNumber(ulBatteryLevel, LL_LED_Panel_GetColorIdxOfYELLOW()); LL_LED_Panel_SingleFrameBufDisplay();
    } else if(ulBatteryLevel < 100) { 
        //LL_LED_Panel_SingleFrameBufInit(); LL_LED_Panel_SingleFrameBufSetNumber(ulBatteryLevel, LL_LED_Panel_GetColorIdxOfGREEN()); LL_LED_Panel_SingleFrameBufDisplay();
    } else {
        //LL_LED_Panel_SetAnimation(LL_LED_PANEL_ANIMATION_GREEN_BATTERY, E_LL_ANIMATION__RE_TRIGGER_TYPE__NOT_IF_SAME);
    }                    
}


unsigned long LL_BatteryIndicator_PowerOnIsFinished(void) {
    if(E_STEP__BATTERY_INDICATOR__RUNTIME__CHK_BATT == step) { return 1; } else { return 0; }
}

static unsigned long top_battery_level = 0xFFFFFFFF;
void LL_BatteryIndicator__Mainloop(void) { // unsigned char global_brightness = gtPara.brightness[gtSysState.eModeOfWarningLight];
    if(E_STEP__BATTERY_INDICATOR__OFF == step) return;
    
//    unsigned long battery_sample = LL_Battery_Sample(); if(LL_ADC_SAMPLE_VALUE_NONE == battery_sample) { return; } // not ready yet
    unsigned long battery_level  = LL_Battery_Level();  if(LL_ADC_SAMPLE_VALUE_NONE == battery_level)  { return; } // not ready yet
    if(0xFFFFFFFF == top_battery_level) { top_battery_level = battery_level; if(100 < top_battery_level) { top_battery_level = 100; } }
    
    switch(step) {
        case E_STEP__BATTERY_INDICATOR__OFF:
            break;
        case E_STEP__BATTERY_INDICATOR__PWR_ON__CHK_BATT:
            if(1 == gtPara.ulNeedCharge) { // has been low-battery and not charge yet
                LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_N__LED_POWER_ENABLE);
                step = E_STEP__BATTERY_INDICATOR__PWR_ON__ALERT_BATT; delay_cnt = gulTimerCnt1ms--; rep_cnt = 0;
            } else if( 53 < battery_level ) { 
                step = E_STEP__BATTERY_INDICATOR__PWR_ON__START_UP; delay_cnt = gulTimerCnt1ms; rep_cnt = 0;
            } else if( 5 < battery_level ) { 
                step = E_STEP__BATTERY_INDICATOR__PWR_ON__START_UP; delay_cnt = gulTimerCnt1ms; rep_cnt = 0;
            } else { gtPara.ulNeedCharge = 1; gulFlashStoreNeeded = 1;  // low-battery, need charge
                LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_N__LED_POWER_ENABLE);
                step = E_STEP__BATTERY_INDICATOR__PWR_ON__ALERT_BATT; delay_cnt = gulTimerCnt1ms--; rep_cnt = 0;
            }
            break;
        case E_STEP__BATTERY_INDICATOR__PWR_ON__START_UP:
            if( 50*45 < LL_Timer_Elapsed_ms(delay_cnt) ) { delay_cnt = gulTimerCnt1ms;
                step = E_STEP__BATTERY_INDICATOR__PWR_ON__FLASH_BATT; delay_cnt = gulTimerCnt1ms--; rep_cnt = 0;
            }
            break;
        case E_STEP__BATTERY_INDICATOR__PWR_ON__FLASH_BATT:
            if( 140 < LL_Timer_Elapsed_ms(delay_cnt) ) { delay_cnt = gulTimerCnt1ms;
                //LL_LED_Panel_SetBrightness(LL_LED_Panel_BRIGHTNESS_OF_BATTERY_INDICATOR);
                if(0 == (rep_cnt%2)) {
                    //LL_BatteryIndicator__ShowInPanel(top_battery_level);//LL_LED_Panel_SingleFrameBufSetNumber(top_battery_level, LL_LED_Panel_GetColorIdxOfRED()); 
                } else { 
                    //LL_LED_Panel_SetAnimation(LL_LED_PANEL_ANIMATION_OFF, E_LL_ANIMATION__RE_TRIGGER_TYPE__NOT_IF_SAME);
                }
                rep_cnt++; if(6 <= rep_cnt) { 
                //LL_LED_Panel_SetBrightness(gtPara.brightness[gtSysState.eModeOfWarningLight]);//(100); // resume to 100%
                step = E_STEP__BATTERY_INDICATOR__RUNTIME__CHK_BATT; delay_cnt = gulTimerCnt1ms--; rep_cnt = 0; }
            }
            break;
        case E_STEP__BATTERY_INDICATOR__RUNTIME__CHK_BATT: // check battery
            if((LL_BATTERY_LEVEL_NONE != battery_level)
            && (                    1 >= battery_level) ) { 
                gtPara.ulNeedCharge = 1; gulFlashStoreNeeded = 1;
                step = E_STEP__BATTERY_INDICATOR__RUNTIME__ALERT_BATT; delay_cnt = gulTimerCnt1ms--; rep_cnt = 0;  
                // off the timeslot to make beep stable:
                sd_radio_session_close();
                // switch off other routine of LED/buzzer:
                LL_BeepForTurning__OFF();        // LL_Helmet_BeepWhenTurning()
                gtSysState.eBrakeState = BRAKE_OFF;     // LL_Helmet_TurnOffBrakeLightAuto()
                gtSysState.eTurnState = TURNING_NONE;   // LL_Helmet_TurnOffTurningLightAuto()
            }
            break;
        case E_STEP__BATTERY_INDICATOR__PWR_ON__ALERT_BATT:
        case E_STEP__BATTERY_INDICATOR__RUNTIME__ALERT_BATT:
            
            if( 140 < LL_Timer_Elapsed_ms(delay_cnt) ) { delay_cnt = gulTimerCnt1ms;
                if(0 == (rep_cnt%2)) {
                    LL_PWM_ON(E_LL_PWM_BUZZER);
                    //LL_LED_Panel_SetAnimation(LL_LED_PANEL_ANIMATION_OFF, E_LL_ANIMATION__RE_TRIGGER_TYPE__NOT_IF_SAME);
                } else { 
                    LL_PWM_OFF(E_LL_PWM_BUZZER);
                    //LL_LED_Panel_SingleFrameBufInit(); LL_LED_Panel_SingleFrameBufSetNumber(0, LL_LED_Panel_GetColorIdxOfRED()); LL_LED_Panel_SingleFrameBufDisplay();
                }
                rep_cnt++; if(10 <= rep_cnt) { 
                    LL_PWM_OFF(E_LL_PWM_BUZZER);
                    step = E_STEP__BATTERY_INDICATOR__OFF; LL_Helmet_ChangeStateTo_OFF(); }
            }
            break;
    }
}


