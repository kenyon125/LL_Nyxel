/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Timer.h"
#include "LL_Power.h"

static unsigned long gulCntIdle = 0;
static unsigned long gulSleepTimeDelta = 0;
void LL_AutoSleep_Delay(unsigned long ms)
{
    gulCntIdle = gulTimerCnt1ms;
    gulSleepTimeDelta = ms;
}

void LL_AutoSleep_Mainloop(void)
{
    if( gulSleepTimeDelta <= LL_Timer_Elapsed_ms(gulCntIdle) ) {
        LL_Power_BeforeSleep();        
        LL_Power_Sleep();
    }
}
