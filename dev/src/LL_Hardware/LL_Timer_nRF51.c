/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "nrf_drv_timer.h"
#include "LL_Timer.h"

const nrf_drv_timer_t TIMER_1ms = NRF_DRV_TIMER_INSTANCE(0); // Note: the TIMER0_ENABLED should be set to 1 in nrf_drv_config.h.

//unsigned long gulTimerCnt200us;
unsigned long gulTimerCnt1ms;
void timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            #if 1
            gulTimerCnt1ms++;
//          LL_Timer_ISR_1ms();
            #else
            gulTimerCnt200us++;
            if( 5<=gulTimerCnt200us ) { gulTimerCnt200us = 0; gulTimerCnt1ms++; }
//          LL_Timer_ISR_200us();
            #endif
            break;
        
        default:
            //Do nothing.
            break;
    }    
}

void LL_Timer_Init(void)
{
    uint32_t time_ticks;
    nrf_drv_timer_init(&TIMER_1ms, NULL, timer_handler);
    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_1ms,   1);  
//  time_ticks = nrf_drv_timer_us_to_ticks(&TIMER_1ms, 200);  
    nrf_drv_timer_extended_compare(&TIMER_1ms, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

void LL_Timer_Enable(void)
{
    nrf_drv_timer_enable(&TIMER_1ms);    
}
