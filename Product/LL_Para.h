/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_PARA_H
#define _LL_PARA_H



#include "LL_Platform.h"
#include "LL_Common.h"
#include "LL_SysMode.h"

// 各种设备的可存储数量:
#define DEVICE_NUM_OF_XXX    2  // 可根据requirement修改, 仅受限于flash空间. (不受限于运行时的地址匹配, 因为运行时只会检测最近一次配对的地址, 不会遍历所有地址的列表.)

// 蓝牙设备的地址长度:
#define BLE_DEVICE_ADDR_LEN 6   // 此值要遵守蓝牙协议, 不可随意修改!

// 参数结构
typedef struct
{
    T_LL_HardwareID HardwareID[DEVICE_NUM_OF_XXX]; 
//  unsigned long ulRoundRobin;
        unsigned char ulRoundRobin;
        unsigned char bits_isCheapRemote;
        unsigned char rsv0;
        unsigned char rev1;
    unsigned long ulIng;
}T_DeviceLumen;

typedef enum
{
    E_BEEP_MODE_OF_POWER_ON_OFF__MIN = 22,
    E_BEEP_MODE_OF_POWER_ON_OFF__MUTE,
    E_BEEP_MODE_OF_POWER_ON_OFF__BEEP,
    E_BEEP_MODE_OF_POWER_ON_OFF__MAX
} E_BEEP_MODE_OF_POWER_ON_OFF;
typedef enum {
    E_TURN_SIGNAL_ANIMATION_TYPE__BLINK = 0,
    E_TURN_SIGNAL_ANIMATION_TYPE__WAVE,
    E_TURN_SIGNAL_ANIMATION_TYPE__NUM,
} E_TURN_SIGNAL_ANIMATION_TYPE;

//#define LL_PARA_VERSION 0x01010101
//#define LL_PARA_VERSION 0x02020202 // add light: PCB Red
//#define LL_PARA_VERSION 0x03030303 // discard "brightness" then add "brightness_individual"
//#define LL_PARA_VERSION 0x04040404 // add new out-of-box (from Meixuan)
  #define LL_PARA_VERSION 0x12//0x05050505 // support 4KB more code flash
#define LL_LED_ANIMATION_MODE_MAX_NUM 30

typedef enum {
    E_FRAME_SPEED_HIGH = 1, //25ms
    E_FRAME_SPEED_MEDIUM = 2,   //50ms
    E_FRAME_SPEED_PAIRING = 3,  //75ms(speical for pairing)
    E_FRAME_SPEED_NORMAL = 4,   //100ms
    E_FRAME_SPEED_SLOW = 8,     //200ms
} E_FRAME_SPEED_OF_ANIMATION;

typedef struct
{
    unsigned long para_version;
    // 已注册的设备列表:
    T_DeviceLumen tRemote;
//  T_DeviceLumen tBrakeL;
//  T_DeviceLumen tBrakeR;
    // 已配对的BLE设备:
    signed char scBleAddr[BLE_DEVICE_ADDR_LEN];
    // 当前闪烁模式:
    E_ModeOfWarningLight eModeOfWarningLight;
    
    //animation para
    E_Ws2812_Animation_Type ePatternOfFrontLight[E_MODE_OF_WARNING_LIGHT__NUM];
    E_Ws2812_Animation_Type ePatternOfRearLight[E_MODE_OF_WARNING_LIGHT__NUM];
//    unsigned int ulFrontColorType[E_MODE_OF_WARNING_LIGHT__NUM]; // 0x FF(frame speed level)  FFFFFF(color)
//    unsigned char ulFrontFrameSpeedLevel[E_MODE_OF_WARNING_LIGHT__NUM];
    
//    unsigned int ulRearColorType[E_MODE_OF_WARNING_LIGHT__NUM];  
//    unsigned char ulRearFrameSpeedLevel[E_MODE_OF_WARNING_LIGHT__NUM];
       
    // brake:
    unsigned long ulBrakeFunction;  // 0-OFF   1-ON
    // beep mode:
    unsigned long eBeepMode;  // 0-OFF   1-ON
    // power off when low battery:
    unsigned long ulNeedCharge;
    // para of AccGyro: K of x/y/z
    signed short ssKxyz[3];
    signed short ssThreshhold;
    unsigned short usAdcValueOfFullCharge;
    // SN
    unsigned char aucSN[20];
    
    // customer flashing mode
    unsigned long customer_flashing_timeslot_states[E_MODE_OF_WARNING_LIGHT__NUM][E_LL_PWM_CH_NUM];   
    // individual brightness
    unsigned long brightness_individual[E_MODE_OF_WARNING_LIGHT__NUM][E_LL_PWM_CH_NUM];    
    
    // beep mode of power on/off
    E_BEEP_MODE_OF_POWER_ON_OFF beep_mode_of_power_on_off;
    // flash erase flag
    unsigned long flash_erase_flag;
    // animation cmds fron app
    //T_AnimationCmds animation_cmds[E_MODE_OF_WARNING_LIGHT__NUM];

    // Brightness of front light and rear panel
    unsigned char brightness[E_MODE_OF_WARNING_LIGHT__NUM];
    E_TURN_SIGNAL_ANIMATION_TYPE turn_signal_animation_type[E_MODE_OF_WARNING_LIGHT__NUM];
    T_BUZZER_BEEP beep_of_turning[E_BEEP_OF_TURNING__TOTAL];
    
    // 校验码:
    unsigned long ulChecksum;
		
	#define GET_BAR_STATE(states, bar_index)    ((states>>(bar_index<<1)) & 0x3) // 2 bit for each bar
		
    unsigned char publicNetworkEnabled;
    unsigned char netID[LL_ESB_BASE_ADDR_LEN];
    unsigned char netID_exist;
    unsigned char syncPowerOff;		
    
    unsigned char glBatteryRank;
		bool glFactoryMode;
}T_Para;
extern T_Para gtPara;

// For the flashing pattern, we have a new feature "double click to sync the pattern to others, but not overwrite their actual one in the flash".
// So, need add a temporary variable to store this "pattern from others" and be used by LED flashing module.
unsigned long LL_Para__get_flashing_pattern(unsigned long mode, E_LL_LED light);
void LL_Para__set_flashing_pattern_fromButtonSwitch(void);
void LL_Para__set_flashing_pattern_fromApp(unsigned long flashing_pattern, unsigned long mode, E_LL_LED light);
void LL_Para__set_flashing_pattern_fromSync(unsigned long flashing_pattern, unsigned long mode, E_LL_LED light);
void LL_Para_store(void);

// for LumOS
#define setPara(name, value)    gtPara.name = value; gulFlashStoreNeeded = 1;
#define getPara(name)           gtPara.name

#endif
