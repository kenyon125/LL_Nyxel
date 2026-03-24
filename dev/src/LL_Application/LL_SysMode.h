/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_SYSMODE_H
#define _LL_SYSMODE_H

#include "LL_LED_Panel_WS2812.h"

typedef enum
{
    SYS_INIT = 0,                   // software initial.
    SYS_PCB_TESTING_MODE,           // PCB test
    SYS_PRODUCTION_TESTING_MODE,    // production test
    SYS_BEEP_BEFORE_ON,
    SYS_ON,
    //SYS_BEEP_BEFORE_PAIRING,
    SYS_PAIRING,
    SYS_BEEP_BEFORE_OFF,
    SYS_OFF,                        // OFF at once if trigger key released.
}E_SYS_ONOFF;

typedef enum {
    E_MODE_OF_WARNING_LIGHT__Mode1 = 0,
    E_MODE_OF_WARNING_LIGHT__Mode2,
    E_MODE_OF_WARNING_LIGHT__Mode3,
    E_MODE_OF_WARNING_LIGHT__NUM,
}E_ModeOfWarningLight;

typedef enum
{
    TURNING_NONE = 0,
    TURNING_L,
    TURNING_R
}E_TURN_STATE;

typedef enum
{
    ADAPTEBOARD_CONNECT=0,
    ADAPTEBOARD_DISCONNECT,
}ADAPTEBOARD_PARA;

typedef enum
{
    BRAKE_OFF = 0,
    BRAKE_ON
}E_BRAKE_STATE;

typedef enum
{
    BEEP_WHEN_TURNING_ONOFF = 0,
    BEEP_EVERY_1_TURNING_FLASH,
    BEEP_EVERY_4_TURNING_FLASH,
    BEEP_EVERY_8_TURNING_FLASH,
    BEEP_NONE,
    BEEP_MODE_MAX
}E_BEEP_MODE;

typedef struct
{
    E_SYS_ONOFF eOnOFF;
    E_ModeOfWarningLight eModeOfWarningLight;
    E_TURN_STATE eTurnState;
    E_BRAKE_STATE eBrakeState;
    unsigned long ulNoConnectionSinceSysOn; // for ESB, no need for BLE because there is only one BLE paired device. 
}T_SysState;

typedef struct
{
    unsigned short gulWs2812FrameFrontTotal;
    unsigned short gulWs2812FrameRearTotal;
    unsigned short gulWs2812FrameDisplayCnt;  
    unsigned int animationFramesFront_flash[LL_ANIMATION_FRAME_NUM][LL_LED_MATRIX_WIDTH];
    unsigned int animationFramesRear_flash[LL_ANIMATION_FRAME_NUM][LL_LED_MATRIX_WIDTH];
}T_PanelPara;    

typedef struct {
    unsigned char eMode;
    unsigned char part;
    E_Ws2812_Animation_Type type; // E_Animation_Type
    unsigned long color_code;
    unsigned long flash_pattern;
} T_Animation_Data;
        
extern unsigned char glAdapterBoardConnState;
extern T_SysState gtSysState;
extern T_SysState gtSysState_prev;
extern T_PanelPara gtPanelPara;

void LL_Helmet_ChangeStateTo_Init(void);
void LL_Helmet_ChangeStateTo_PcbTestingMode(void);
void LL_Helmet_ChangeStateTo_ProductionTestingMode(void);
void LL_Helmet_ChangeStateTo_BeepBeforeON(void);
void LL_Helmet_ChangeStateTo_ON(void);
void LL_Helmet_ChangeStateTo_NextMode(void);
void LL_Helmet_ChangeStateTo_BeepBeforePairing(void);
void LL_Helmet_ChangeStateTo_Pairing(void);
void LL_Helmet_ChangeStateTo_BeepBeforeOFF(void);
void LL_Helmet_ChangeStateTo_OFF(void);
void LL_Helmet_Mainloop(void);

//void LL_Helmet_RedGreenFlashWhenTestMode_OFF(void);
//void LL_Helmet_RedGreenFlashWhenTestMode_ON(void);
//unsigned long LL_Helmet_RedGreenFlashWhenTestMode_isON(void);



extern unsigned long gulCmdTestingMode;
extern unsigned int glPowerOnDisplayNum;
extern bool glPowerOnAnimationPlayOff;
#define LL_CMD_TESTING_BATT      1
#define LL_CMD_TESTING_DEVID     2
#define LL_CMD_TESTING_RMTID     3
#define LL_CMD_TESTING_RBATT     4
#define LL_CMD_TESTING_FWVER     5
#define LL_CMD_TESTING_WSNH      6
#define LL_CMD_TESTING_WSNL      7
#define LL_CMD_TESTING_RSNH      8
#define LL_CMD_TESTING_RSNL      9
#define LL_CMD_TESTING_BUZZ     10
void LL_Helmet_CmdTestingMode_CMD(unsigned long ulCmd);



void LL_BeepForTurning__OFF(void);
void LL_BeepForTurning__ON(void);



#endif
