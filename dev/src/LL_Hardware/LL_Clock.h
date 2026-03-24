/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_CLOCK_H
#define _LL_CLOCK_H



// NOTE: no XTAL/RC selection of H/L-FREQ-CLK yet!

/*******************************************************************************
@brief 
@details
@param[in ] none
@param[out] none
@retval     none
*******************************************************************************/
void LL_Clock_Init(void);



// a stable clock for LED
//
// get its clock value
unsigned long   LL_Clock__LED_CLOCK_get(void);
// convert the clock value to ms
#define         LL_Clock__LED_CLOCK_TO_MS(clock_value)  ((clock_value) >> 5) // 5: 2^5 = 32 = 32768Hz / 1000ms



// for sync
//
// snapshot other's (master's) clock when receiving it
void          LL_Clock__OTHERS_LED_CLOCK_snapshot( unsigned long nowLedClock_others );
// use other's (master's) clock (to do flashing patter) to get the sync effect
unsigned long LL_Clock__OTHERS_LED_CLOCK_get(void);
// clear the snapshot if self is master (no need use other's clock)
void          LL_CLOCK__clearTheSnapshotAsMaster(void);
// update the snapshot as a slave
void          LL_CLOCK__updateTheSnapshotAsSlave(unsigned long clock_self, unsigned long clock_others);



#endif
