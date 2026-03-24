/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "LL_Hardware_cfg.h"
#include "LL_Power.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_PWM.h"
#include "LL_LED_Helmet.h"
#include "LL_Para.h"
#include "LL_Clock.h"

unsigned long gulTimerCnt1ms_LED;

typedef struct {
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
} T_LL_LED;

static T_LL_LED gtLED[E_LL_PWM_CH_NUM];

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

void LL_LEDs_OFF(void) {
    memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));
    memset(gtPanelPara.animationFramesRear_flash, 0,sizeof(gtPanelPara.animationFramesRear_flash));
    for(int i=0; i<LL_PWM_CH_NUM; i++) {
        LL_LED_OFF((E_LL_LED)i);
    }
}
void LL_LEDs_init(void) {
    // turn off all the LEDs
    LL_LEDs_OFF();
    // set default brightness
    for(int i=0; i<LL_PWM_CH_NUM; i++) {
        if(E_LL_PWM_BUZZER == i) {
            LL_PWM_DutySet(i, LL_PWM_FREQ_OF_BUZZER);
        } else {
            LL_PWM_DutySet(i, 100);
        }
    }  
}
void LL_LEDs_Sleep(void) {
    // turn off all the LEDs
    LL_LEDs_OFF();
}

static uint16_t last_displayed_frame = 0;  
static uint32_t last_frame_timestamp = 0;
void LL_LEDs_routine(void) {
    unsigned long time_snapshot = LL_Clock__LED_CLOCK_TO_MS(LL_Clock__OTHERS_LED_CLOCK_get());//(NRF_RTC1->COUNTER + gulTimerCntDiffFromSyncMaster) / 33; // 33 = 32768Hz / 1000ms kzc

    unsigned long total_duration = 2000; 
    float frame_duration = (float)total_duration / 48;
    uint16_t current_frame = (uint16_t)((time_snapshot % total_duration) / frame_duration);

    if(glPowerOnAnimationPlayOff == false){// need display that starting at the first frame
        extern unsigned long ulPoweronDisplayTime;
        if(50 <= LL_Timer_Elapsed_ms(ulPoweronDisplayTime)) { ulPoweronDisplayTime = gulTimerCnt1ms;
            LL_Drv_Ws2812_Front_Draw(gtPanelPara.gulWs2812FrameDisplayCnt);
            LL_Drv_Ws2812_Rear_Draw(gtPanelPara.gulWs2812FrameDisplayCnt);
            LL_Drv_Ws2812_Display();
            
            if(gtPanelPara.gulWs2812FrameDisplayCnt < LL_ANIMATION_FRAME_NUM - 5){ //can not set LL_ANIMATION_FRAME_NUM, will appear white flash when power on
                gtPanelPara.gulWs2812FrameDisplayCnt++;
            }else{
                glPowerOnAnimationPlayOff = true;
                memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));
                memset(gtPanelPara.animationFramesRear_flash, 0,sizeof(gtPanelPara.animationFramesRear_flash));
            }    
        }
    }else{
        if(current_frame >= 48){current_frame = 47;}  
        if (current_frame != last_displayed_frame && time_snapshot - last_frame_timestamp >= frame_duration) 
        {
            last_displayed_frame = current_frame;
            last_frame_timestamp = time_snapshot;

            LL_Drv_Ws2812_Front_Draw(current_frame);
            LL_Drv_Ws2812_Rear_Draw(current_frame);

            if (LL_Drv_Ws2812_Display() != NRF_SUCCESS) {
            }
        }
    }
    
    //For Buzzer
     for(int i=0; i<E_LL_PWM_CH_NUM; i++) { //global_brightness = gtPara.brightness_individual[gtSysState.eFlashTypeOfBackLight][i];
        if(2 == gtLED[i].ulON) {
            unsigned long ulDurationON, ulDurationOFF, ulDurationALL;
            ulDurationON  = gtLED[i].ulFlashPeriodOn;
            ulDurationOFF = gtLED[i].ulFlashPeriodOff;
            ulDurationALL = ulDurationON + ulDurationOFF;
            if(time_snapshot % ulDurationALL <= ulDurationON) { 
                LL_PWM_ON(i); 
            } else {
                LL_PWM_OFF(i); 
            }   
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

            duration_now = LL_Timer_Elapsed_ms(gtLED[i].ulCntFlash); while(duration_now >= duration_total) {
                duration_now -= duration_total;
                gtLED[i].ulCntFlash += duration_total; 
            }
       
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
        }
    }
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

    #if 1 // do not change the freq
        if(LL_PWM_PERIOD_us(1) == buzzer_period_us) { // if 1Hz, must be "car blinker"
            LL_LED_Brightness(E_LL_PWM_BUZZER, 100); // 100: no wave here, so that can "bi" by the low frequence flashing pattern
            {
                gtLED[eLED].ulFlashPeriodOn = TOTAL_DURATION__car_blinker; 
                gtLED[eLED].ulFlashPeriodOff = (unsigned long)(&beep_car_blinker); // the pointer of 6 segments
                gtLED[eLED].ulON = 5; // 5: the LED flashing type is "special for buzzer"
                gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately            
            }
            return; // skip the next config
        } else if(LL_PWM_PERIOD_us(4) == buzzer_period_us) { // else if 4Hz, must be "woodcock"
            LL_LED_Brightness(E_LL_PWM_BUZZER, 100); // 100: no wave here, so that can "bi" by the low frequence flashing pattern
            {
                gtLED[eLED].ulFlashPeriodOn = TOTAL_DURATION__woodcock;
                gtLED[eLED].ulFlashPeriodOff = (unsigned long)(&beep_woodcock); // the pointer of 6 segments
                gtLED[eLED].ulON = 5; // 5: the LED flashing type is "special for buzzer"
                gtLED[eLED].ulCntFlash = gulTimerCnt1ms; // start immediately            
            }
            return; // skip the next config
        } else { // else, normal buzzer
            LL_LED_Brightness(E_LL_PWM_BUZZER,  50); //  50: any value between 0~100 is ok, just need a wave.        
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
