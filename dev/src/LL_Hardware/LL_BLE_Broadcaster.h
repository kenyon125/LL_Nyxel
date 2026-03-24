/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_BLE_Broadcaster_H
#define _LL_BLE_Broadcaster_H // BLE role "Broadcaster" to manage all advertisement (for Lumos APP and other Lumos devices)



// User this module step-by-step
//
// 1) Device Type: pls update this value acc to "Lumos Interface for Partners (BLE Advertising, V1.4)"
//#define LL_BLE_Broadcaster__Device_Type 0xB0 // 0xB0: Device Type "Controller"
  #define LL_BLE_Broadcaster__Device_Type 0x08 // 0x08 / 0x09: Nyxel Standard / quin
//
// 2) Advertising Parameter: no need modify usually.
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Pairing                100, 180 // ms,s
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Lumos_Lights_State     100, 180 // ms,s
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Battery                100, 180 // ms,s
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Sync_Clock            1000,   1 // 1000ms and 1s: means ONLY ADV 1 TIME
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Power_ONOFF            100, 180 // ms,s
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_TurningAndBraking      100, 180 // ms,s
#define LL_BLE_Broadcaster__interval_and_timeout_of_MSG_FlashingPattern        100, 180 // ms,s
//
// 3) Put this func to the main loop:
void LL_BLE_Broadcaster(void);
//
// 4) Start one of these threads:
extern const unsigned long thread__LL_BLE_Broadcaster__Idle[];
extern const unsigned long thread__LL_BLE_Broadcaster__Pairing_APP[];
extern const unsigned long thread__LL_BLE_Broadcaster__Pairing_Devices[];
extern const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode[];
extern const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_TurningBraking[];
extern const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Sync_FlashingPattern[];
extern const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Power_ON[];
extern const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Power_OFF[];
void LL_BLE_Broadcaster__start(const unsigned long *thread_xxx);



#endif
