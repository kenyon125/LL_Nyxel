/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Hardware_cfg.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_Key.h"



T_LL_KEY_CFG gatKeyCfg[LL_KEY_NUM_MAX] = LL_KEY_CFG;
static unsigned long sgaulDbnCnt[        LL_KEY_NUM_MAX ];   // every key has a debounce counter.
static unsigned long sgaulState[         LL_KEY_NUM_MAX ];   // every key has a state bak.
static unsigned long sgaulIgnoreRelease[ LL_KEY_NUM_MAX ];   // whether ignore the next release after a meaningful long press.
static T_LL_KEY_EVT sgtEvt = {0xFFFFFFFF, 0};

void LL_Key_Init(void) //(unsigned long N)
{
    for( unsigned long N = 0; N < LL_KEY_NUM_MAX; N++ ) {
        LL_GPIO_InputCfg(gatKeyCfg[N].port, gatKeyCfg[N].pin, gatKeyCfg[N].pull, gatKeyCfg[N].trigger);
        sgaulDbnCnt[N] = gulTimerCnt1ms;   
        sgaulState[N] = gatKeyCfg[N].normal;
    }
}

void LL_Key_Init_With_No_Trigger(void) //(unsigned long N)
{
    for( unsigned long N = 0; N < LL_KEY_NUM_MAX; N++ ) {
        LL_GPIO_InputCfg(gatKeyCfg[N].port, gatKeyCfg[N].pin, gatKeyCfg[N].pull,LL_GPIO_TRIGGER_NONE );
    }
}
static unsigned long sgulOneKeyPerScan = 0xFFFFFFFF;
void LL_Key_Scan(void) //(unsigned long N)
{    
    sgulOneKeyPerScan++; 
    if(LL_KEY_NUM_MAX <= sgulOneKeyPerScan) sgulOneKeyPerScan = 0;
    unsigned long N = sgulOneKeyPerScan;
    
    unsigned long ulPinValue = LL_GPIO_InputRead(gatKeyCfg[N].port, gatKeyCfg[N].pin);
    if(sgaulState[N] == ulPinValue) {  // Likely: unchanged.
        // clear the debounce counter:
        sgaulDbnCnt[N] = gulTimerCnt1ms;
        return;
    } else {   // key state change.
        // check debounce:
        if( LL_Timer_Elapsed_ms(sgaulDbnCnt[N]) < gatKeyCfg[N].debounce ) {  // debouncing.
            return;
        } else {   // key action!
            // clear the debounce counter:
            sgaulDbnCnt[N] = gulTimerCnt1ms;                        
            // update key state:
            sgaulState[N] = ulPinValue;
            // return a message:
            if( gatKeyCfg[N].normal == ulPinValue ) { 
                if(0 == sgaulIgnoreRelease[N]) {   sgtEvt.N = N; sgtEvt.evt = 0; }
            } else {    sgaulIgnoreRelease[N] = 0; sgtEvt.N = N; sgtEvt.evt = 1; }
        }
    }                  
}

unsigned long LL_Key_EvtFetch(T_LL_KEY_EVT *ptEvt)
{
    if( 0xFFFFFFFF == sgtEvt.N ) { return 0; }  // likely
    else { 
        ptEvt->N   = sgtEvt.N;   sgtEvt.N = 0xFFFFFFFF;
        ptEvt->evt = sgtEvt.evt;
        return 1;
    }
}
unsigned long LL_Key_EvtPeek( T_LL_KEY_EVT *ptEvt)
{
    if( 0xFFFFFFFF == sgtEvt.N ) { return 0; }  // likely
    else { 
        ptEvt->N   = sgtEvt.N;
        ptEvt->evt = sgtEvt.evt;
        return 1;
    }
}
void LL_Key_IgnoreNextRelease(unsigned long N)
{
    sgaulIgnoreRelease[N] = 1;
}
