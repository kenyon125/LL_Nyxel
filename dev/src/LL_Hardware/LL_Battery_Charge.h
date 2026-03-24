/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_BATTERY_CHARGE_H
#define _LL_BATTERY_CHARGE_H

unsigned long LL_Battery_Charging__NotOnlyChargingMode_ButAlsoChargingIndeed(void);
unsigned long LL_Battery__isCharging(void);

void LL_Battery_Charging__Init(void);
void LL_Battery_Charging__Loop(void);



#endif
