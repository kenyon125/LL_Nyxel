/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_BATTERYINDICATOR_H
#define _LL_BATTERYINDICATOR_H



/*******************************************************************************
 @brief        : battery indicator.
 @details      : none
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       : none
*******************************************************************************/
void            LL_BatteryIndicator__Start(void);
void            LL_BatteryIndicator__Stop(void);
unsigned long   LL_BatteryIndicator__isStopped(void);
void            LL_BatteryIndicator__Mainloop(void);



#endif
