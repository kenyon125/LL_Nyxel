/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#include "LL_Hardware_cfg.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_PWM.h"
#include "LL_LED_Helmet.h"
#include "LL_Battery_Charge.h"
#include "LL_Para.h"
#include "LL_Clock.h"

unsigned long gulTimerHappened_1xxus = 0;
unsigned long gulTimerCnt1ms_LED;
#define LL_Timer_Elapsed_ms_LED(cnt_prev)  ( ((cnt_prev)<=(gulTimerCnt1ms_LED)) ? ((gulTimerCnt1ms_LED)-(cnt_prev)) : (0xFFFFFFFF-(cnt_prev)+(gulTimerCnt1ms_LED)) )


typedef struct
{
    // hardware
////unsigned long ulPin;
////unsigned long ulPush;
////unsigned long ulRealState;      // 0 means OFF, 1 means ON.
    // software state
    unsigned long ulON;             // 0 means OFF, 1 means ON, 2 means flash, 3 means customer flashing, 4 means fade on, 
                                    // 5&6 means 5-segment buzzer, 0xB means breathing.
    unsigned long ulFlashPeriodOn ; // For mode 2: the "flash on " time, unit: ms.
                                    // For mode 4: a cnt for fade on.
                                    // For mode 0xB: a cnt for breathing.
    unsigned long ulFlashPeriodOff; // when flashing, the "off" time, unit: ms.
                                    // For mode 5&6: the 6th segment.
                                    // For mode 0xB: a cnt for breathing.
    unsigned long ulBrightness;
    // counter
    unsigned long ulCntFlash;       // for flashing.
////unsigned long ulCntPwm;         // for PWM.
}T_LL_LED;
T_LL_LED gtLED[LL_LED_NUM];


void LL_LED_OFF(E_LL_LED eLED)
{
    LL_PWM_OFF(eLED);
    gtLED[eLED].ulON = 0;
}
void LL_LED_ON( E_LL_LED eLED)
{
    LL_PWM_ON(eLED);
    gtLED[eLED].ulON = 1;
}

void LL_LED_Brightness( E_LL_LED eLED, unsigned long ulBrightness )
{
		gtLED[eLED].ulBrightness = ulBrightness;
    LL_PWM_DutySet(eLED, ulBrightness);
}
void LL_LED_Brightness_noSafeDuty( E_LL_LED eLED, unsigned long ulBrightness )
{
    gtLED[eLED].ulBrightness = ulBrightness;
    LL_PWM_DutySet_noSafeDuty(eLED, ulBrightness);
}

void LL_LED_Flashing(E_LL_LED eLED, unsigned long ulOn, unsigned long ulOff)
{
    if(2 != gtLED[eLED].ulON ) { gtLED[eLED].ulCntFlash = gulTimerCnt1ms_LED; }//{ gtLED[eLED].ulCntFlash = gulTimerCnt1ms; }
    gtLED[eLED].ulON = 2;
    gtLED[eLED].ulFlashPeriodOn  = ulOn;
    gtLED[eLED].ulFlashPeriodOff = ulOff;
}
void LL_LED_FlashingWithLeadingOff(E_LL_LED eLED, unsigned long ulOn, unsigned long ulOff, unsigned long leading_off_ms)
{
    if(2 != gtLED[eLED].ulON ) { gtLED[eLED].ulCntFlash = gulTimerCnt1ms_LED; gtLED[eLED].ulCntFlash -= (ulOn + ulOff - leading_off_ms); }
    gtLED[eLED].ulON = 2;
    gtLED[eLED].ulFlashPeriodOn  = ulOn;
    gtLED[eLED].ulFlashPeriodOff = ulOff;
}

void LL_LED_FlashingByCustomer(E_LL_LED eLED)
{
    gtLED[eLED].ulON = 3;
}


//
#define BUZZER_BEEP__car_blinker {   \
    .freq = 1,   \
    .slots_duration = {10,10, 10,10, 10,10},    \
    .need_loop = 1,  \
}
#define TOTAL_DURATION__car_blinker     (500*6) // total: 500ms * 6 segments
T_BUZZER_BEEP beep_car_blinker = BUZZER_BEEP__car_blinker;
//
#define BUZZER_BEEP__woodcock {   \
    .freq = 4,   \
    .slots_duration = {2,2, 2,2, 2,(4000/50 - 2*5)},    \
    .need_loop = 1,  \
}
#define TOTAL_DURATION__woodcock     4000 // total: 4000ms
T_BUZZER_BEEP beep_woodcock = BUZZER_BEEP__woodcock;
//

void LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_LED eLED, T_BUZZER_BEEP *ptBeep)
{
    unsigned long buzzer_period_us = LL_PWM_PERIOD_us(ptBeep->freq); 
    #if 0 // change freq every time
        if(1) { //(LL_PWM__GetBuzzerPeriod() != buzzer_period_us) {
            LL_PWM_Sleep(); LL_PWM_Init(buzzer_period_us); LL_LED_Brightness(E_LL_LED_BUZZER, 50); // 50: any value between 0~100 is ok, just need a wave.        
        }
    #endif
    #if 1 // do not change the freq
        if(LL_PWM_PERIOD_us(1) == buzzer_period_us) { // if 1Hz, must be "car blinker"
            LL_LED_Brightness(E_LL_LED_BUZZER, 100); // 100: no wave here, so that can "bi" by the low frequence flashing pattern
            {
                gtLED[eLED].ulFlashPeriodOn = TOTAL_DURATION__car_blinker; 
                gtLED[eLED].ulFlashPeriodOff = (unsigned long)(&beep_car_blinker); // the pointer of 6 segments
                gtLED[eLED].ulON = 5; // 5: the LED flashing type is "special for buzzer"
                gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately            
            }
            return; // skip the next config
        } else if(LL_PWM_PERIOD_us(4) == buzzer_period_us) { // else if 4Hz, must be "woodcock"
            LL_LED_Brightness(E_LL_LED_BUZZER, 100); // 100: no wave here, so that can "bi" by the low frequence flashing pattern
            {
                gtLED[eLED].ulFlashPeriodOn = TOTAL_DURATION__woodcock;
                gtLED[eLED].ulFlashPeriodOff = (unsigned long)(&beep_woodcock); // the pointer of 6 segments
                gtLED[eLED].ulON = 5; // 5: the LED flashing type is "special for buzzer"
                gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately            
            }
            return; // skip the next config
        } else { // else, normal buzzer
            LL_LED_Brightness(E_LL_LED_BUZZER,  50); //  50: any value between 0~100 is ok, just need a wave.        
        }
    #endif
    //
//    if(not_wait_the_1st_segment) {
//        gtLED[eLED].ulON = 6;
//        gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately
//    } else {
//        gtLED[eLED].ulON = 5;
//    }
//    gtLED[eLED].ulFlashPeriodOff = ms_of_the_6th_off_segment;
    //
    unsigned long duration_total = 0; for(int slot = 0; slot < BEEP_SLOT_TOTAL_NUMBER; slot++) {
        duration_total += ptBeep->slots_duration[slot];
    } duration_total *= BEEP_SLOT_DURATION_UNIT; if(duration_total == 0) { LL_LED_OFF(eLED); return; }
    gtLED[eLED].ulFlashPeriodOn = duration_total;
    //
    gtLED[eLED].ulFlashPeriodOff = (unsigned long)ptBeep;
    //
    gtLED[eLED].ulON = 5;//if(ptBeep->need_loop) { gtLED[eLED].ulON = 6; } else { gtLED[eLED].ulON = 5; }
    gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately
}

//void LL_LED_FADE_ON(E_LL_LED eLED) {
//    gtLED[eLED].ulON = 4;
//    gtLED[eLED].ulFlashPeriodOn = gulTimerCnt1ms;
//    LL_PWM_DutySet(eLED, 0);
//    LL_PWM_ON(eLED);
//}

void LL_LED_BREATHING(E_LL_LED eLED) { if(gtLED[eLED].ulON == 0xB) return; // if already, exit
    gtLED[eLED].ulON = 0xB;
    gtLED[eLED].ulFlashPeriodOn  = gulTimerCnt1ms;
    gtLED[eLED].ulFlashPeriodOff = 0;
    LL_PWM_DutySet(eLED, 0);
    LL_PWM_ON(eLED);
}

void LL_LEDs_init(void)
{
    int i;
        
    for(i=0; i<LL_LED_NUM; i++)
    {
        if(E_LL_LED_BUZZER == i) {
            LL_PWM_DutySet(i, LL_PWM_FREQ_OF_BUZZER);
        } else {
            LL_LED_Brightness((E_LL_LED)i, 100);
        }
        LL_LED_OFF((E_LL_LED)i);            
    }
}


void LL_LEDs_routine_100us(void)
{    
    gulTimerCnt1ms_LED = gulTimerCnt1ms;// unsigned char global_brightness;// = gtPara.brightness[gtSysState.eFlashTypeOfBackLight];
   unsigned long time_snapshot = LL_Clock__LED_CLOCK_TO_MS(LL_Clock__OTHERS_LED_CLOCK_get());//(NRF_RTC1->COUNTER + gulTimerCntDiffFromSyncMaster) / 33; // 33 = 32768Hz / 1000ms
    for(int i=0; i<LL_LED_NUM; i++) { //global_brightness = gtPara.brightness_individual[gtSysState.eFlashTypeOfBackLight][i];
        if(2 == gtLED[i].ulON) {
            #if 0 // Ultra original
            if(      LL_Timer_Elapsed_ms_LED(gtLED[i].ulCntFlash) < gtLED[i].ulFlashPeriodOn                             ) { LL_PWM_ON(i); }
            else if( LL_Timer_Elapsed_ms_LED(gtLED[i].ulCntFlash) < gtLED[i].ulFlashPeriodOn + gtLED[i].ulFlashPeriodOff ) { LL_PWM_OFF(i); }
            else                                                                                                           { gtLED[i].ulCntFlash += gtLED[i].ulFlashPeriodOn + gtLED[i].ulFlashPeriodOff; }//{ gtLED[i].ulCntFlash = gulTimerCnt1ms_LED; }
            #else // Sync
                unsigned long ulDurationON, ulDurationOFF, ulDurationALL;
                ulDurationON  = gtLED[i].ulFlashPeriodOn;
                ulDurationOFF = gtLED[i].ulFlashPeriodOff;
                ulDurationALL = ulDurationON + ulDurationOFF;
                if(time_snapshot % ulDurationALL <= ulDurationON) { 
                    LL_PWM_ON(i); 
                } else {
                    LL_PWM_OFF(i); 
                }
            #endif
        } else if(3 == gtLED[i].ulON) {
//            #if 0 // 12 timeslot and variable time length
//            unsigned long timeslot_len    = gtPara.customer_flashing_timeslot_len[gtPara.eModeOfWarningLight][i]; 
//            #else // 24 timeslot and fixed time length
//            unsigned long timeslot_len    = 2000/24; // 2000ms 24 slot
//            #endif
//            unsigned long timeslot_seq = (gulTimerCnt1ms % (timeslot_len*LL_LED_CUSTOMER_FLASHING_TIMESLOT_NUM)) / timeslot_len;
//            if(LL_LED_CUSTOMER_FLASHING_TIMESLOT_NUM <= timeslot_seq) { return; } // exception!
//            else {
//                if(0 != (timeslot_states&(1<<timeslot_seq))) { LL_PWM_ON(i); } else { LL_PWM_OFF(i); }
//            }
            // according to product team:
            unsigned long total_duration     = 2*1000; // 2s
            unsigned long total_bar_num      = 12;
            unsigned long total_timeslot_num = 48; // to support 1/48
            //
            unsigned long index_of_now_bar      = (time_snapshot % total_duration) * total_bar_num      / total_duration; if(index_of_now_bar      >= total_bar_num     ) { index_of_now_bar      = total_bar_num      - 1; }
            unsigned long index_of_now_timeslot = (time_snapshot % total_duration) * total_timeslot_num / total_duration; if(index_of_now_timeslot >= total_timeslot_num) { index_of_now_timeslot = total_timeslot_num - 1; }
            unsigned long index_in_bar_of_now_timeslot = index_of_now_timeslot % (total_timeslot_num / total_bar_num);
            //
            // get the bar setting from flash
            unsigned long setting_of_all_bar = LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, (E_LL_LED)i);// gtPara.customer_flashing_timeslot_states[gtPara.eModeOfWarningLight][i];
            unsigned long setting_of_now_bar = GET_BAR_STATE(setting_of_all_bar, index_of_now_bar);
            //
            unsigned long need_ON = 0;
            switch(setting_of_now_bar) {
                case 0: { // OFF
                    // nothing to do
                break; }
                case 1:
                case 2: { // half blink
                    #define _LL_DEMO_1of48_BLINKING
                    #ifdef _LL_DEMO_1of48_BLINKING
                    { // half blink means 1/48 blink
                        if(0 == index_in_bar_of_now_timeslot) { need_ON = 1; }
                    }
                    #else
                    { // half blink means 1/24 blink
                        if( (0 == index_in_bar_of_now_timeslot) || (1 == index_in_bar_of_now_timeslot) ) { need_ON = 1; }
                    }
                    #endif
                break; }
                case 3: { // ON
                    need_ON = 1;
                break; }
                default: { // will not happen
                    // TO DO: must be stack overflow!
                break; }
            }
            if(need_ON) { LL_PWM_ON(i); } else { LL_PWM_OFF(i); }
            
//        } else if(4 == gtLED[i].ulON) {
//            unsigned long last_timer_cnt = gtLED[i].ulFlashPeriodOn;
//            if( 5 <= LL_Timer_Elapsed_ms(last_timer_cnt) ) { gtLED[i].ulFlashPeriodOn = gulTimerCnt1ms_LED;
//                unsigned long ulBrightness = LL_PWM_DutyGet(i); if(ulBrightness < global_brightness) { ulBrightness++; }
//                LL_PWM_DutySet(i, ulBrightness);
//            }    
        } else if( (5 == gtLED[i].ulON) || (6 == gtLED[i].ulON)) {
            unsigned long duration_total = gtLED[i].ulFlashPeriodOn;
            T_BUZZER_BEEP *ptBeep = (T_BUZZER_BEEP *)(gtLED[i].ulFlashPeriodOff);
            //
            unsigned long last_slot = BEEP_SLOT_TOTAL_NUMBER - 1; 
            unsigned long duration[BEEP_SLOT_TOTAL_NUMBER]; 
            unsigned long temp_duration; for(int slot = 0; slot < BEEP_SLOT_TOTAL_NUMBER; slot++) { temp_duration = ptBeep->slots_duration[slot];
                if(0 == temp_duration) { // if this duratin is 0
                    last_slot = slot-1; break; // the last slot, no need continue
                } else { // else, this duration is not 0
                    duration[slot] = temp_duration * BEEP_SLOT_DURATION_UNIT;
                }
            }
            //
            unsigned long duration_now;
            #if 0
            { // aligned
                duration_now = gulTimerCnt1ms % duration_total;
            }
            #else
            { // not aligned
                duration_now = LL_Timer_Elapsed_ms(gtLED[i].ulCntFlash); while(duration_now >= duration_total) {
                    duration_now -= duration_total;
                    gtLED[i].ulCntFlash += duration_total; 
                }
            }
            #endif        
            //
            unsigned long which_slot_now, duration_added = 0; for(int slot = 0; slot < BEEP_SLOT_TOTAL_NUMBER; slot++) { duration_added += duration[slot];
                if(duration_now < duration_added) { // if in this slot
                    which_slot_now = slot; break; // found, exit
                } else { // else, not in this slot
                    continue; // continue finding
                }
            }
            //
            unsigned long is_a_beep_OFF_slot = which_slot_now & 0x1; if(is_a_beep_OFF_slot) { LL_PWM_OFF(i); } else { LL_PWM_ON(i); }
            if((last_slot == which_slot_now) && (is_a_beep_OFF_slot)) {
                if(ptBeep->need_loop) { LL_PWM_OFF(i); } else { LL_LED_OFF((E_LL_LED)i); }
            }
            
        } else if(0xB == gtLED[i].ulON) { unsigned long max_brightness = 100;//global_brightness;
            #ifdef _LL_DEMO_CHARGING_RED_LIGHT_BREATHING_AND_SLEEPING
            { // ON 2s, then loop the "breath, OFF 1s"
                //
                unsigned long duration_per_brightness = 10; // 10ms
                //
                unsigned long duration_of_beginning_on  = 2*1000; // 2s
                unsigned long duration_of_breathing_off = duration_per_brightness*max_brightness;
                unsigned long duration_of_off           = 1000; // 1s
                unsigned long duration_of_breathing_on  = duration_of_breathing_off; // same as off
                unsigned long duration_total_of_loop    = duration_of_breathing_off + duration_of_off + duration_of_breathing_on;
                //
                unsigned long brightness;
                #define SET_BRIGHTNESS(i, brightness)  if(gtLED[i].ulFlashPeriodOff != brightness) { LL_PWM_DutySet(i, brightness); gtLED[i].ulFlashPeriodOff = brightness; }
                unsigned long time_interval = LL_Timer_Elapsed_ms(gtLED[i].ulFlashPeriodOn);
                if(time_interval <= duration_of_beginning_on) { // if beginning ON stage
                    brightness = max_brightness;
                } else { // else, must be loop stage
                    unsigned long time_interval_in_loop = (time_interval - duration_of_beginning_on) % duration_total_of_loop;
                    if(time_interval_in_loop <= duration_of_breathing_off) { // if breathing OFF stage
                        brightness = time_interval_in_loop / duration_per_brightness; brightness = max_brightness - brightness;
                    } else if(time_interval_in_loop <= (duration_of_breathing_off + duration_of_off)) { // else, if 1s OFF stage
                        brightness = 0;
                    } else { // else, must be breathing ON stage
                        brightness = (time_interval_in_loop - duration_of_breathing_off - duration_of_off) / duration_per_brightness;
                    }
                }
                //
                if(brightness != 0) {
                    SET_BRIGHTNESS(i, brightness)
                    LL_PWM_ON(i);
                } else {
                    LL_PWM_OFF(i);
                }
            }
            #else
            {
                unsigned long last_timer_cnt = gtLED[i].ulFlashPeriodOn;
                if( 5 <= LL_Timer_Elapsed_ms(last_timer_cnt) ) { gtLED[i].ulFlashPeriodOn = gulTimerCnt1ms_LED;
                    //
                    unsigned long last_brightness_cnt = gtLED[i].ulFlashPeriodOff; 
                    if(last_brightness_cnt <= (global_brightness<<1)) {
                        last_brightness_cnt++; 
                    } else {
                        last_brightness_cnt = 0; 
                    } gtLED[i].ulFlashPeriodOff = last_brightness_cnt;
                    //
                    unsigned long brightness;
                    if(last_brightness_cnt <= global_brightness) { 
                        brightness = last_brightness_cnt;
                    } else { //if(last_brightness_cnt <= (global_brightness<<1)) {
                        brightness = global_brightness - (last_brightness_cnt - global_brightness);
                    }
                    LL_PWM_DutySet(i, brightness);
                }    
            }
            #endif
        }
    }
}

void LL_LEDs_OFF(void)
{
    int i;   
    for(i=0; i<LL_LED_NUM; i++)
    {
        LL_LED_OFF((E_LL_LED)i);
    }
}

void LL_LEDs_PowerOff_BeforeSleep(void)
{
    int i;
//    LL_GPIO_OutputWrite(0, PIN_LL_LED_BELT_POWER, LL_LED_BELT_POWER_NORMAL); //NRF_GPIO->OUTCLR = 1<<PIN_LL_LED_BELT_POWER;
    for(i=0; i<LL_LED_NUM; i++) { 
        LL_LED_OFF((E_LL_LED)i);
    }    
}
