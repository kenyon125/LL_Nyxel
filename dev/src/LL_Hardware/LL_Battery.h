/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_BATTERY_H
#define _LL_BATTERY_H


extern unsigned long gulBatteryLowIndicateAfterPowerOn;

extern unsigned long gulBatteryOfRemote;

/*******************************************************************************
 @brief        : Battery functions initialization.
 @details      : none
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       : none
*******************************************************************************/
void LL_Battery_Init(void);

/*******************************************************************************
 @brief        : Battery functions mainloop.
 @details      : none
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       : none
*******************************************************************************/
void LL_Battery_Mainloop(void);

/*******************************************************************************
 @brief       
 @details      : none
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       :
*******************************************************************************/
#define LL_BATTERY_LEVEL_NONE   0xFFFFFFFF
unsigned long LL_Battery_Level(void);   // return: 1~100, or LL_BATTERY_LEVEL_NONE.



#endif


