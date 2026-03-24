/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_LED_HELMET_H
#define _LL_LED_HELMET_H



#include "LL_Hardware_cfg.h"

#if 1 // buzzer
//
typedef struct {
    // freq
    #define IS_BEEP_FREQ_VALID(freq)    ( (((freq)>=1)&&((freq)<=20000)) ? 1 : 0 )  // 1Hz ~ 20KHz, why 20K?
    #define DEFAULT_BUZZER_FREQ         1600    // 2KHz, from the buzzer datasheet
    unsigned short freq;
    //
    // slots
    #define BEEP_SLOT_TOTAL_NUMBER      6   // 6 slots required by PM
    #define BEEP_SLOT_DURATION_UNIT     50//100 // 100ms, the unit of slot duration "XY" in uart COMMAND "XY-XY-XY-XY-XY", duration less than 100ms can not be counted precisely by firmware.
    unsigned char slots_duration[BEEP_SLOT_TOTAL_NUMBER];
    //
    unsigned char need_loop;
} T_BUZZER_BEEP;
//
// several types
#define BUZZER_BEEP__NONE   {   \
    .freq = DEFAULT_BUZZER_FREQ,   \
    .slots_duration = {0,0,0,0,0,0},    \
    .need_loop = 0,  \
}
#define BUZZER_BEEP__ONCE   {   \
    .freq = DEFAULT_BUZZER_FREQ,   \
    .slots_duration = {1*2,1*2,0,0,0,0},    \
    .need_loop = 0,  \
}
#define BUZZER_BEEP__PER_0p5S   {   \
    .freq = DEFAULT_BUZZER_FREQ,   \
    .slots_duration = {1*2, 4*2, 1*2, 4*2, 1*2, 4*2},    \
    .need_loop = 1,  \
}
#define BUZZER_BEEP__PER_2S   {   \
    .freq = DEFAULT_BUZZER_FREQ,   \
    .slots_duration = {1*2,19*2,  1*2,19*2,  1*2,19*2},    \
    .need_loop = 1,  \
}
#define BUZZER_BEEP__TWICE_PER_2S   {   \
    .freq = DEFAULT_BUZZER_FREQ,   \
    .slots_duration = {1*2,1*1,  1*2,17*2,  0,0},    \
    .need_loop = 1,  \
}
#define BUZZER_BEEP__1HZ    {   \
    .freq = 1,   \
    .slots_duration = {1*2,0,0,0,0,0},    \
    .need_loop = 1,  \
}
#define BUZZER_BEEP__WOODCOCK   {   \
    .freq = 4,   \
    .slots_duration = {15*2,25*2, 15*2,25*2, 15*2,25*2},    \
    .need_loop = 1,  \
}
//
typedef enum {
    E_BEEP_OF_TURNING__L_BEGIN = 0,
    E_BEEP_OF_TURNING__R_BEGIN,
    E_BEEP_OF_TURNING__L_END,
    E_BEEP_OF_TURNING__R_END,
    E_BEEP_OF_TURNING__TOTAL,
} E_BEEP_OF_TURNING;
//
#endif

// flashing time, unit: ms.
// flashing time, unit: ms.
#define LL_LED_BACK_SLOW_FLASH_ON   500 //600//150//300
#define LL_LED_BACK_SLOW_FLASH_OFF  500 //400//100//200
#define LL_LED_BACK_FAST_FLASH_ON   125 //150// 75//150
#define LL_LED_BACK_FAST_FLASH_OFF  125 //100// 50//100
#define LL_LED_TURN_FLASH_ON        250
#define LL_LED_TURN_FLASH_OFF       (500 - LL_LED_TURN_FLASH_ON)
//#define LL_LED_BEEP_FLASH_ON         80
//#define LL_LED_BEEP_FLASH_OFF       1520

// single LED
void LL_LED_OFF(E_LL_LED eLED);
void LL_LED_ON( E_LL_LED eLED);
void LL_LED_Flashing(E_LL_LED eLED, unsigned long ulOn, unsigned long ulOff); // ulOn/ulOff: in unit of ms.
#define LL_LED_CUSTOMER_FLASHING_TIMESLOT_NUM    24 // 24 timeslot totally.
void LL_LED_FlashingByCustomer(E_LL_LED eLED);
void LL_LED_FADE_ON(E_LL_LED eLED);
void LL_LED_HeartBeat(E_LL_LED eLED);

// whole LED module
void LL_LEDs_OFF(void);
void LL_LEDs_init(void);
void LL_LEDs_Sleep(void);
void LL_LEDs_routine(void);
void LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_LED eLED, T_BUZZER_BEEP *ptBeep);//void LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_LED eLED, unsigned long ms_of_the_6th_off_segment, unsigned long not_wait_the_1st_segment);




#endif
