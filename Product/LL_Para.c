/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Board.h"
#include "LL_Flash.h"
#include "LL_Para.h"

T_Para gtPara;
// For the flashing pattern, we have a new feature "double click to sync the pattern to others, but not overwrite their actual one in the flash".
// So, need add a temporary variable to store this "pattern from others" and be used by LED flashing module.
//
static unsigned long customer_flashing_timeslot_states[E_MODE_OF_WARNING_LIGHT__NUM][E_LL_PWM_CH_NUM]; static unsigned long is_temp_flashing_pattern_enabled = 0;
//
// need a disable/enable for this temporary, since it may have conflict with the patterns stored in the flash
static unsigned long LL_Para__is_flashing_pattern_temp_enabled(void) {
    return is_temp_flashing_pattern_enabled;
}
static void LL_Para__disable_flashing_pattern_temp(void) {
    is_temp_flashing_pattern_enabled = 0;
}
static void LL_Para__enable_flashing_pattern_temp(void) {
    is_temp_flashing_pattern_enabled = 1;
}
//
// get/set
static unsigned long LL_Para__get_flashing_pattern_temp(unsigned long mode, E_LL_LED light) {
    return customer_flashing_timeslot_states[mode][light];
}
static void LL_Para__set_flashing_pattern_temp(unsigned long flashing_pattern, unsigned long mode, E_LL_LED light) {
    customer_flashing_timeslot_states[mode][light] = flashing_pattern;
    LL_Para__enable_flashing_pattern_temp();
}
//
//
//
// a get/set for external, the "set" has several variant
unsigned long LL_Para__get_flashing_pattern(unsigned long mode, E_LL_LED light) {
    unsigned long current_pattern;
    if(LL_Para__is_flashing_pattern_temp_enabled()) {
        current_pattern = LL_Para__get_flashing_pattern_temp(mode, light);
    } else {
        current_pattern = gtPara.customer_flashing_timeslot_states[mode][light];
    }
    return current_pattern;
}
void LL_Para__set_flashing_pattern_fromButtonSwitch(void) {
    // just disable the temporary
    LL_Para__disable_flashing_pattern_temp();
}
void LL_Para__set_flashing_pattern_fromApp(unsigned long flashing_pattern, unsigned long mode, E_LL_LED light) {
    gtPara.customer_flashing_timeslot_states[mode][light] = flashing_pattern; gulFlashStoreNeeded = 1;
    LL_Para__disable_flashing_pattern_temp();//customer_flashing_timeslot_states[mode][light] = flashing_pattern;
}
void LL_Para__set_flashing_pattern_fromSync(unsigned long flashing_pattern, unsigned long mode, E_LL_LED light) {
    LL_Para__set_flashing_pattern_temp(flashing_pattern, mode, light);
    LL_Para__enable_flashing_pattern_temp();
}


static inline char LL_ChkParaSize()   
{                                                   
    char dummy[LL_Flash_Size_of_Para + 1];        
    LL_ChkParaSize(); // call itself to avoid unused warning. 
    return dummy[sizeof(T_Para)];  // If warning, means a fatal error that para size is larger than flash.
}

unsigned long gulFlashStoreNeeded = 0;
static unsigned long gulFlashStoreCnt;
void LL_Para_store(void) {
    if(0 == gulFlashStoreNeeded) {
        return;
    } else if(1 == gulFlashStoreNeeded) { 
        gulFlashStoreCnt = gulTimerCnt1ms;
        gulFlashStoreNeeded = 2;
    } else if(2 == gulFlashStoreNeeded) {
        if(2000 <= LL_Timer_Elapsed_ms(gulFlashStoreCnt)) { //gulFlashStoreCnt = gulTimerCnt1ms;
            LL_Flash_store();
            gulFlashStoreNeeded = 0; 
        }                            
    } else { // exception
        gulFlashStoreNeeded = 0;         
    }
}
/*
void LL_Para_UpdateAdcValueOfFullCharge(unsigned long ulAdcValue)
{
    gtPara.usAdcValueOfFullCharge = (unsigned short)ulAdcValue;
    gulFlashStoreNeeded = 1;
}

unsigned long LL_Para_GetAdcValueOfFullCharge(void)
{
    return (unsigned long)gtPara.usAdcValueOfFullCharge;
}
*/
