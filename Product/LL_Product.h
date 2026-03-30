/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_Product_H
#define _LL_Product_H



#define LL_BLE_TX_POWER         4 //BLE tx power: -30, -20, -16, -12, -8, -4, 0, and 4 dBm.
#define LL_BLE_ADV_NAME         "Nyxel"
#define LL_BLE_MANU_NAME        "LumenLabs"
#define LL_BLE_ADV_INTERVAL     500 // ms, 20ms ~ 10.24s



// Device Type: pls update this value acc to "Lumos Interface for Partners (BLE Advertising, V1.4)"
//#define LL_Device_Type 0xB0 // 0xB0: Device Type "Controller"
  #define LL_Device_Type 0x07 // 0x06 / 0x07: Ultra / Ultra E-Bike



//#define LL_VER_H        2
//#define LL_VER_L        0
//
// Vx.yz
#define LL_VER_BLE_DIS  "V2.13"  // BLE DIS service, length ?
#define LL_VER_x        2
#define LL_VER_y        1
#define LL_VER_z        3
//
// Date when building
#define LL_VER_DATE     0xA31E  // 0xYMDD: Y(0~F) is 2016~2031, M(1~C) is Jun~Dec, DD(01~1F) is 1~31.



#define LL_Mode_Number  3



//#define LL_Charging_Lights_Controlled_By_Software   1   // like: Ultra
  #define LL_Charging_Lights_Controlled_By_Hardware   1   // like: UEB



//#define LL_A_POWER_CONTROL_FOR_ALL_LED  1



#define LIGHTS_LoopedWhenTestingPCB         \
                    E_LL_LED_TURN_L,        \
                    E_LL_LED_TURN_R,        \
                    E_LL_LED_FrontWhiteL,   \
                    E_LL_LED_FrontWhiteR,   \
                    E_LL_LED_FRONT,         \
                    E_LL_LED_FrontYellowR,  \
                    E_LL_LED_FrontYellowL,  \
                    E_LL_LED_BACK_R,        \
                    E_LL_LED_BACK_L
#define LIGHTS_FlashingWhenPairing          \
                    E_LL_LED_FRONT,         \
                    E_LL_LED_TURN_L,        \
                    E_LL_LED_TURN_R,        \
                    E_LL_LED_FrontWhiteL,   \
                    E_LL_LED_FrontWhiteR



#define LL_FLASH_PAGE_NUM_FOR_USER_SETTING_SO_THAT_BOOTLOADER_WONT_OVERWRITE 1



// Fill in these parts:
//
// main_init0_ASAP: Why ASAP? Like: turn off the light, or: some operation required by hardware.
// main_init1_BeforeFlash: means the persistent data has not been loaded from flash yet
// main_init2_Flash: to load the persistent data 
// main_init3_AfterFlash: means the persistent data has been loaded and can be used now
//
// main_loop



#endif


