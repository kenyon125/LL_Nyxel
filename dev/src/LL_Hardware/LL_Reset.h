/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_RESET_H
#define _LL_RESET_H



// return: reason defined by chip vender.
unsigned long LL_Reset_Reasons(void);

/*
// 4 beep to indicate: ("S" means short beep, "L" means long.)
// SSSS - power on
// 
void LL_Reset_ReasonsIndicateByBeep(void);
//void LL_Reset_ReasonsIndicateByLEDs(void);
*/



#endif
