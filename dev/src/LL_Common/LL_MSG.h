/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_MSG_H
#define _LL_MSG_H



#include "LL_HardwareID.h"

#define LL_VER_H        2
#define LL_VER_L        1
#define LL_VER_DATE     0xA305  // 0xYMDD: Y(0~F) is 2016~2031, M(1~C) is Jun~Dec, DD(01~1F) is 1~31.
#define LL_VER_BLE_DIS  "V2.10"  // BLE DIS service, length ?

typedef struct
{
    T_LL_HardwareID tHardwareID;    // ID of remote.
    char cStateTurn; // as a remote: 'O' means no turning;  'L' means turning left;  'R' means turning right.
    char cStateBrake;  // as a brake: 'K' means braking;  'N' means no braking; "C" means "Cheap Remote".
    char cBattery;
    signed short ssAcc[3];
    char cBrakeFuncON;  // [0]: 0/1 means OFF/ON. [3..1]: ver_H.  [7..4]: ver_L.
                        // Only used by test DK now, not used by helmet yet.
    char cKxyz[3];
    signed short ssThreshhold;
    unsigned short usVerDate;   // LL_VER_DATE
}T_MSG_toHelmet; // size: 26
extern T_MSG_toHelmet sgtMsg;//gtMsgToHelmet;    // make it global for relay to use it

typedef struct
{
    T_LL_HardwareID tHardwareID;    // ID of remote.
    char cBrakeFuncON;              // 1£ºON; 0: OFF.
    signed short ssKxyz[3];
    signed short ssThreshhold;
}T_MSG_toRemote; // size: 18
void LL_MSG_toRemote_pack(void);



// "mode << 5": Bit[7..5] of the 2nd byte is "mode"
// "31": means Flashing Pattern of Front Light
#define LL_MSG__PACK_FlashingPatternOfFrontLight(buffer, mode, pattern) \
    buffer[0] = 'F';\
    buffer[1] = (mode << 5) + 31;\
    buffer[2] = (pattern & 0x000000FF) >>  0;\
    buffer[3] = (pattern & 0x0000FF00) >>  8;\
    buffer[4] = (pattern & 0x00FF0000) >> 16;   
#define LL_MSG__PACK_FlashingPatternOfLight(buffer, light, mode, pattern) \
    buffer[0] = 'F';\
    buffer[1] = (mode << 5) + light;\
    buffer[2] = (pattern & 0x000000FF) >>  0;\
    buffer[3] = (pattern & 0x0000FF00) >>  8;\
    buffer[4] = (pattern & 0x00FF0000) >> 16;   

#define LL_MSG__PACK_Brightness(buffer, mode, brightness) \
    buffer[0] = 'B';\
    buffer[1] = 'M';\
    buffer[2] = mode;\
    buffer[3] = brightness;



// if message is 10 bytes AND has prefix "PROFILE ", may be "PROFILE nn" while "nn" is command
//
// 1) 1st edition
//                    Chen Long  2:59 PM
//                    front | PROFILE 00
//                    front left | PROFILE 01
//                    front right | PROFILE 02
//                    rear | PROFILE 03
//                    rear left | PROFILE 04
//                    rear right | PROFILE 05 (edited) 
//
//                    PROFILE ??     
//
// 2) added "00" as "factory default"
//                    PROFILE nn, while "nn" can be:
//                    00: factory default (same as "front")
//                    01: front
//                    02: front left
//                    03: fornt right
//                    04: rear
//                    05: rear left
//                    06: rear right
//                    ??: app gets current profile from firmware                    
//                    
typedef enum {
    E_Profile_Default = 0,
    E_Profile_Front,
    E_Profile_FrontLeft,
    E_Profile_FrontRight,
    E_Profile_Rear,
    E_Profile_RearLeft,
    E_Profile_RearRight,
    E_Profile_LeftOnly,
    E_Profile_RightOnly,
    TOTAL_PROFILE_NUM
} E_Profile;
extern char profile_names[TOTAL_PROFILE_NUM][20+1]; // 7 profiles totally, 20 is the max length of Nordic BLE UART, 1 is for end symbol "\0"
extern unsigned char onoff_Front[ TOTAL_PROFILE_NUM ];
extern unsigned char onoff_Rear[  TOTAL_PROFILE_NUM ];
extern unsigned char onoff_L[     TOTAL_PROFILE_NUM ];
extern unsigned char onoff_R[     TOTAL_PROFILE_NUM ];
extern unsigned long the_current_profile_support_braking(void);



#endif
