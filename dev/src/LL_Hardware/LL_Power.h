/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_POWER_H
#define _LL_POWER_H



/*******************************************************************************
@brief      Pls call it when power on.
@param[in]  void
@param[out] void
@retval     void
@details    none
*******************************************************************************/
void LL_Power_Init(void);

/*******************************************************************************
@brief      Pls realize it yourself! It'll be called before sleep.
@details
@param[in ] none
@param[out] none
@retval     none
*******************************************************************************/
void LL_Power_BeforeSleep(void);
//void LL_Power_BeforeWakeup(void);

/*******************************************************************************
@brief      Pls call it when sleep directly.
@param[in]  void
@param[out] void
@retval     void
@details    none
*******************************************************************************/
void LL_Power_Sleep(void);



#endif
