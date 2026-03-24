/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_AUTOSLEEP_H
#define _LL_AUTOSLEEP_H



/*******************************************************************************
@brief      Call it to delay the auto-sleep.
@details
@param[in ] none
@param[out] none
@retval     none
*******************************************************************************/
void LL_AutoSleep_Delay(unsigned long ms);

/*******************************************************************************
@brief      Pls call it in main loop.
@param[in]  void
@param[out] void
@retval     void
@details    none
*******************************************************************************/
void LL_AutoSleep_Mainloop(void);



#endif
