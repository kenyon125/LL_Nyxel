/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "nrf.h" // included for RTC
#include "LL_Clock.h"

void LL_Clock_Init(void)
{
    // Pls see ble_stack_init()
#if 0 // Xiong: 不能使用默认的XTAL, 会导致P0.26和P0.27不可用.
    clock_lf_cfg.source = NRF_CLOCK_LF_SRC_RC;  
    clock_lf_cfg.rc_ctiv = 1;
    clock_lf_cfg.rc_temp_ctiv = 1;
#endif
}



// Use NRF_RTC1 for LED module since it's more stable than software one "gulTimerCnt1ms":
//
// get
#define LED_CLOCK_MAX   0x00FFFFFF  // 24-bit
unsigned long LL_Clock__LED_CLOCK_get(void) {
    return NRF_RTC1->COUNTER;
}
//
// interval & clock calculation
static unsigned long LL_Clock__LED_CLOCK_interval(unsigned long begin, unsigned long end) {
    unsigned long interval;
    if(end >= begin) {
        interval = end - begin;
    } else { // else, "end" is smaller, must be overflow
        interval = (LED_CLOCK_MAX - begin) + end;
    }
    return interval;
}
static unsigned long LL_Clock__LED_CLOCK_calculate(unsigned long since, unsigned long interval) {
    return ((since + interval) & LED_CLOCK_MAX);
}



// snapshot the current clock of both self and others(sync master)
static unsigned long snapshot_self;
static unsigned long snapshot_others;
void LL_Clock__OTHERS_LED_CLOCK_snapshot(unsigned long nowLedClock_others) {
    snapshot_self   = LL_Clock__LED_CLOCK_get();
    snapshot_others = nowLedClock_others;
}
// clear the snapshot as a master
void LL_CLOCK__clearTheSnapshotAsMaster(void) {
    snapshot_self   = 0;
    snapshot_others = 0;
}
// update the snapshot as a slave
void LL_CLOCK__updateTheSnapshotAsSlave(unsigned long clock_self, unsigned long clock_others) {
    snapshot_self   = clock_self;
    snapshot_others = clock_others;
}
// calculate the current time of others(sync master)
unsigned long LL_Clock__OTHERS_LED_CLOCK_get(void) {
    unsigned long nowLedClock_self = LL_Clock__LED_CLOCK_get();
    return LL_Clock__LED_CLOCK_calculate(snapshot_others, LL_Clock__LED_CLOCK_interval(snapshot_self, nowLedClock_self));
}


