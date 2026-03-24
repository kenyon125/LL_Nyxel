/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_TIMER_H
#define _LL_TIMER_H



extern unsigned long gulTimerCnt1ms;
#define LL_Timer_Elapsed_ms(cnt_prev)  ( ((cnt_prev)<=(gulTimerCnt1ms)) ? ((gulTimerCnt1ms)-(cnt_prev)) : (0xFFFFFFFF-(cnt_prev)+(gulTimerCnt1ms)) )


// operation for a cnt which is also a flag of "0 means not work, others means need work"
#define LL_Timer_CntStart(cnt) cnt=(gulTimerCnt1ms-1)|0x1   // record current time, and keep it not 0
#define LL_Timer_CntStop( cnt) cnt=0

/*******************************************************************************
@brief      Pls call it when power on.
@param[in]  void
@param[out] void
@retval     void
@details    none
*******************************************************************************/
void LL_Timer_Init(void);

/*******************************************************************************
@brief      Pls call it to start timer.
@param[in]  void
@param[out] void
@retval     void
@details    none
*******************************************************************************/
void LL_Timer_Enable(void);

#if 0
/*******************************************************************************
@brief      Pls realize it yourself! It'll be called per 1ms.
@param[in]  void
@param[out] void
@retval     void
@details    none 
*******************************************************************************/
void LL_Timer_ISR_1ms(void);
#endif


#endif
