/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Platform.h"
#include "LL_Common.h"
void advertising_init(void);
void update_device_name(const char *new_name);

extern uint16_t m_conn_handle;
extern ble_nus_t  m_nus;

extern T_LL_HardwareID gtHwID;

#define LL_BLE_ADV_PAIRING_FLAG 0x0101
extern void LL_BLE_SetAdManu(unsigned short manu);
extern void battery_level_update(void);

extern bool wake_up_event__power_on_cmd_scanned;
extern bool wake_up_event__key_pressed;

extern uint16_t m_conn_handle;
extern ble_nus_t  m_nus;

extern T_LL_HardwareID gtHwID;

#define LL_BLE_ADV_PAIRING_FLAG 0x0101
extern void LL_BLE_SetAdManu(unsigned short manu);
extern void battery_level_update(void);
extern void advertising_start(void);
       unsigned long sgulGeneralStep;
static unsigned long sgulGeneralCnt;
static unsigned long sgulBeepCnt;
//static unsigned long sgulFrontLightFadeCnt;

extern unsigned long gulFlashStoreNeeded;////LL_Flash_store(); 
unsigned long IsVibrationNeedRunTime;
static unsigned long sgulXxxTestPowerOffStep = 0;
static unsigned long sgulXxxTestPowerOffCnt = 0;
T_SysState gtSysState;
T_SysState gtSysState_prev;
T_PanelPara gtPanelPara;
extern uint16_t m_conn_handle;

#if 1 // Enter pairing-mode instead of power-off, if a Long-Press since power-on.
static unsigned long sgulEnterPairingModeIfLongPress = 0;
#endif

unsigned long IndicatorOfBrakeFuncOnOff__time_cnt = 0;
unsigned long IndicatorOfBrakeFuncOnOff__step_cnt = 0;
void IndicatorOfBrakeFuncOnOff__Start(void) { 
    LL_Timer_CntStart(IndicatorOfBrakeFuncOnOff__time_cnt);
    //
    if(1 == gtPara.ulBrakeFunction) { // brake function is ON

        IndicatorOfBrakeFuncOnOff__step_cnt = 0; 
    } else { // brake function must be OFF
        IndicatorOfBrakeFuncOnOff__step_cnt = 0xFFFFFFFF; 
    }
}

void IndicatorOfBrakeFuncOnOff__Stop( void) { 
    LL_Timer_CntStop(IndicatorOfBrakeFuncOnOff__time_cnt);
//  IndicatorOfBrakeFuncOnOff__step_cnt = 0; 
}
void IndicatorOfBrakeFuncOnOff(void) {
    if(0 == IndicatorOfBrakeFuncOnOff__time_cnt) { return; } // this func is OFF
    switch(IndicatorOfBrakeFuncOnOff__step_cnt) {
        case 0:
            if(2000 < LL_Timer_Elapsed_ms(IndicatorOfBrakeFuncOnOff__time_cnt)) { LL_Timer_CntStart(IndicatorOfBrakeFuncOnOff__time_cnt);
                IndicatorOfBrakeFuncOnOff__step_cnt++;
            }
            break;
        case 1:
            //LL_HelmetActionWhenStateChanged();
            IndicatorOfBrakeFuncOnOff__step_cnt++;
            break;
        default: // exception
            IndicatorOfBrakeFuncOnOff__Stop();
            break;
    }
}

void LL_Helmet_ChangeStateTo_Init()
{
    gtSysState.eOnOFF = SYS_INIT;//SYS_OFF;
    gtSysState.eTurnState = TURNING_NONE;
    gtSysState.eBrakeState = BRAKE_OFF;
    memcpy(&gtSysState_prev, &gtSysState, sizeof(T_SysState));
    // 
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms;    
}
static inline void LL_Helmet_State_Init(void)
{
    T_LL_KEY_EVT tKeyEvt;
    switch(sgulGeneralStep) {
        case  0:
            // if ON/OFF key pressed: check whether a short or long press.
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
                sgulGeneralCnt = gulTimerCnt1ms;
                sgulGeneralStep++; 
            }
            if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { //Prevent the wrong touch of the key to enter this stage and cannot go out
                LL_Power_BeforeSleep(); 
                LL_Power_Sleep();                 
            }            
            break;    
        case  1:
            // if ON/OFF key released: ON!
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) {
                sgulEnterPairingModeIfLongPress = 0;
                LL_Helmet_ChangeStateTo_BeepBeforeON(); 
            } else {
                // if long pressed: enter normal mode, and continue checking whether it's a longer press for pairing-mode.
                if(500 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { //LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                    sgulEnterPairingModeIfLongPress = 1;
                    LL_Helmet_ChangeStateTo_BeepBeforeON(); 
                }
            }
            break;    
    }
    
    #ifdef LL_BatteryIndicator__ShowWhenHelmetNotOn
//        if(1 != LL_Helmet_RedGreenFlashWhenTestMode_isON()) {
            LL_Battery_Charging_Indicator();
//        }    
    #endif    
}


//static unsigned long sgaulSparePin[LL_TEST_MODE_PIN_NUM] = LL_TEST_MODE_SPARE_PINS;
//static unsigned char sgaucTestModePack[16];
//static unsigned long sgulTestModeLedLoopCnt, sgulTestModeLedLoopStep;
void LL_Helmet_ChangeStateTo_PcbTestingMode(void)
{
    gtSysState.eOnOFF = SYS_PCB_TESTING_MODE;
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms; 
    LL_Key_Init_With_No_Trigger();    
    glPowerOnAnimationPlayOff = true;
    
    LL_GPIO_OutputCfg(0, LL_PIN__PCB_BLUE, LL_PULLMODE__PCB_BLUE, LL_PIN_N__PCB_BLUE);
    LL_GPIO_OutputWrite(0, LL_PIN__PCB_BLUE, LL_PIN_Y__PCB_BLUE);
    
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_Y__LED_POWER_ENABLE);
    LL_GPIO_OutputWrite(0, LL_PIN_CHARGING_POWER, LL_PIN_Y__CHARGING_POWER);
       
//  sgulTestModeLedLoopCnt = gulTimerCnt1ms; sgulTestModeLedLoopStep = 0;
//    ble_advertising_start(BLE_ADV_MODE_FAST); //timeslot_sd_start();
}

static inline void LL_Helmet_State_PcbTestingMode(void)
{
    unsigned long battery_level = LL_Battery_Level();
    unsigned long PCBTestColor;
    //charging low level && standby high level  mean charing 
    //charging high level && standby low level  mean full-battery 
    //charging high level && standby high level  mean warning!
                
    //charging 
    unsigned long ulSample_1 = VOLTAGE_OF_AD_VALUE(LL_ADC_Sample_1);
    //charging standby
    unsigned long ulSample_2 = VOLTAGE_OF_AD_VALUE(LL_ADC_Sample_2);    

    if(ulSample_1 > 1000 && ulSample_2 < 100){ PCBTestColor = WS2812_COLOR_GREEN_40;
    }else if(ulSample_1 > 1000 && ulSample_2 > 1000){ PCBTestColor = WS2812_COLOR_YELLOW;//warning
    }else if(ulSample_1 < 100 && ulSample_2 > 1000){ //charging 
        //battery
        if(battery_level <= 20){
            PCBTestColor = WS2812_COLOR_RED_40;
        }else{
            PCBTestColor = WS2812_COLOR_GREEN_40;
        }
    }
    
    for (int i = 0; i < LL_ANIMATION_FRAME_NUM; i++) {
        for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
            gtPanelPara.animationFramesFront_flash[i][j] = PCBTestColor;
            gtPanelPara.animationFramesRear_flash[i][j] = PCBTestColor;
        }
    }
                          
    // wait button event to beep:
    T_LL_KEY_EVT tKeyEvt;
//    if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) ) {
//        if(1 == tKeyEvt.evt) { LL_PWM_ON( E_LL_PWM_BUZZER); }
//        else                 { LL_PWM_OFF(E_LL_PWM_BUZZER); }
//    }
    
    switch(sgulGeneralStep) {
        case  0:
            // if ON/OFF key pressed: check whether a short or long press.
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
                LL_PWM_ON( E_LL_PWM_BUZZER);
                sgulGeneralCnt = gulTimerCnt1ms;
                sgulGeneralStep++; 
            }
            break;    
        case  1:
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) { // if ON/OFF key released
                LL_PWM_OFF(E_LL_PWM_BUZZER);
                sgulGeneralStep = 0; // back to step 0
            } else { // else, not released yet
                // if long pressed: OFF.
                if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                    LL_GPIO_OutputWrite(0, LL_PIN__PCB_BLUE, LL_PIN_N__PCB_BLUE);
                    LL_PWM_OFF(E_LL_PWM_BUZZER);
                    LL_Helmet_ChangeStateTo_BeepBeforeOFF(); 
                }
                // sgulGeneralStep = ?  // no need set "sgulGeneralStep" since it has been set in the "LL_Helmet_ChangeStateTo_" above
            }
            break;
    }
}

bool glPowerOnAnimationPlayOff;
unsigned int glPowerOnDisplayNum;
void LL_Helmet_ChangeStateTo_BeepBeforeON(void)
{
    E_Ws2812_Animation_Type AnimationPowerOnLevel;
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_Y__LED_POWER_ENABLE);
    gtSysState.eOnOFF = SYS_BEEP_BEFORE_ON;
    sgulBeepCnt = gulTimerCnt1ms; 
    uint8_t PowerOnBatteryLevel = LL_Battery_Level();
    if(PowerOnBatteryLevel > 80){ glPowerOnDisplayNum = 28; AnimationPowerOnLevel = LED_ANIMATION_POWERON_LEVEL_4;}
    else if(PowerOnBatteryLevel > 60){ glPowerOnDisplayNum = 21; AnimationPowerOnLevel = LED_ANIMATION_POWERON_LEVEL_3;}
    else if(PowerOnBatteryLevel > 40){ glPowerOnDisplayNum = 18; AnimationPowerOnLevel = LED_ANIMATION_POWERON_LEVEL_2;}
    else{ glPowerOnDisplayNum = 12; AnimationPowerOnLevel = LED_ANIMATION_POWERON_LEVEL_1;}
        
    
    //sgulFrontLightFadeCnt = gulTimerCnt1ms;
    // LED on for both battery-level-measuring and helmet-on-indicator:
    if(1 != gtPara.ulNeedCharge) {      
        glPowerOnAnimationPlayOff = false;
        gtPanelPara.gulWs2812FrameDisplayCnt = 0;
        switch(gtPara.eModeOfWarningLight) {
            case E_MODE_OF_WARNING_LIGHT__Mode1:
            case E_MODE_OF_WARNING_LIGHT__Mode2:
            case E_MODE_OF_WARNING_LIGHT__Mode3:
                //set animation
                LL_Drv_Ws2812_SetFrontAnimation(AnimationPowerOnLevel);
                LL_Drv_Ws2812_SetRearAnimation(AnimationPowerOnLevel);  
                break;
            default:
                
                //set animation
                LL_Drv_Ws2812_SetFrontAnimation(AnimationPowerOnLevel);
                LL_Drv_Ws2812_SetRearAnimation(AnimationPowerOnLevel);  
                break;
        }
    }
    LL_BatteryIndicator__Start();//LL_BatteryIndicatorOfPowerOn_Start();
    
    sgulGeneralCnt = gulTimerCnt1ms;    
  
}

static inline void LL_Helmet_State_BeepBeforeON(void)
{
    T_LL_KEY_EVT tKeyEvt;
    
    if(       125 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { if(E_BEEP_MODE_OF_POWER_ON_OFF__BEEP == gtPara.beep_mode_of_power_on_off) { LL_LED_ON(  E_LL_PWM_BUZZER ); } }
    else if(  250 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { LL_LED_OFF( E_LL_PWM_BUZZER ); }
    else if(  375 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { if(E_BEEP_MODE_OF_POWER_ON_OFF__BEEP == gtPara.beep_mode_of_power_on_off) { LL_LED_ON(  E_LL_PWM_BUZZER ); } }
    else if(  500 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { LL_LED_OFF( E_LL_PWM_BUZZER ); }
    else                                               { 
//      if(LL_BatteryIndicatorOfPowerOn_IsFinished()) {
        if(true) {
            if(glPowerOnAnimationPlayOff == true){    
                if(0 == sgulEnterPairingModeIfLongPress) {
                    LL_Helmet_ChangeStateTo_ON(); 
                } else {
                    if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) {
                        LL_Helmet_ChangeStateTo_ON();                     
                    } else {
                        if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                            #if 1 // Enter pairing-mode instead of power-off, if a Long-Press since power-on.
                            if(0 == sgulEnterPairingModeIfLongPress) {                            
                                LL_Helmet_ChangeStateTo_ON(); 
                            } else { sgulEnterPairingModeIfLongPress = 0;
                                LL_Helmet_ChangeStateTo_Pairing();
                            }
                            #endif
                        }                    
                    }

                }
            }
       }
    }
	//geBroadcastState = E_BROADCAST_STATE__NEED_PTX_OnlyClock; // sync kzc
}

void LL_Helmet_ChangeStateTo_ON(void)
{      
    gtSysState.eOnOFF                   = SYS_ON; //LL_LEDs_PowerOn_AfterWakeUp();
    gtSysState.ulNoConnectionSinceSysOn = 1;
    gtSysState.eModeOfWarningLight    = gtPara.eModeOfWarningLight;
    LL_HelmetActionWhenStateChanged();

//    LL_BLE_SetAdManu(0);
//    battery_level_update();
//    //ble_advertising_start(BLE_ADV_MODE_FAST); timeslot_sd_start();
    //LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_Power_ON);    

    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms;     
}
void LL_Helmet_ChangeStateTo_NextMode(void)
{
    gtSysState.eOnOFF = SYS_ON;
    // switch to next mode
    switch(gtSysState.eModeOfWarningLight){
        case E_MODE_OF_WARNING_LIGHT__Mode1:
            gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode2;
            break;
        case E_MODE_OF_WARNING_LIGHT__Mode2:
            gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3;
            break;
        case E_MODE_OF_WARNING_LIGHT__Mode3:
        default:
            gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode1;
            break;
    }  
    
    // save the new mode to flash
    gtPara.eModeOfWarningLight = gtSysState.eModeOfWarningLight; gulFlashStoreNeeded = 1;

    LL_Para__set_flashing_pattern_fromButtonSwitch(); // disable the "temporary pattern from sync" actually	
    
    gtSysState.eBrakeState  = BRAKE_OFF;	
    LL_HelmetActionWhenStateChanged();
    
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms;     
}

// switch mode when "singl-e click"
// sync when "double click"
#define DOUBLE_CLICK_TIMEOUT    500 // unit: ms
static unsigned long sgulTimerCntPrevRelease = 0;
static unsigned long needSwitchMode = 0;
void TxCurrentFlashModeToApp(void);
static inline void LL_Helmet_State_ON(void)
{
    T_LL_KEY_EVT tKeyEvt;
    switch(sgulGeneralStep) {
        case  0:
            // if ON/OFF key pressed: check whether a short or long press.
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
                sgulGeneralCnt = gulTimerCnt1ms;
                sgulGeneralStep++; 
            }
            break;    
        case  1:
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) { // if ON/OFF key released
                if(DOUBLE_CLICK_TIMEOUT > LL_Timer_Elapsed_ms(sgulTimerCntPrevRelease)) { // if 2nd release in 500ms, which means "double click"
                    needSwitchMode = 0; // cancel the "mode switching when timeout for the 2nd click of double click"
                    {
//                        if(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight] <= LED_ANIMATION_FASTFLASH_MODE){ //common flash pattern
//                            geBroadcastState = E_BROADCAST_STATE__NEED_PTX; // sync
//                        }else{
//                            geBroadcastState = E_BROADCAST_STATE__NEED_PTX_OnlyClock; // sync clock
//                        }
                        //LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_Sync_FlashingPattern);
                    }
                    sgulGeneralStep = 0; // back to step 0
                } else { // else, a single click, or the 1st click of a "double click"
                    needSwitchMode = 1; // switch later because need wait for the possible "double click"
                    sgulGeneralStep = 0; // back to step 0
                }
                sgulTimerCntPrevRelease = gulTimerCnt1ms; // update the "previous release" for next usage
            } else { // else, not released yet
                // if long pressed: OFF.
                if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                    //LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_Power_OFF);
                    if(1 == getPara(syncPowerOff)) {
                                geBroadcastState = E_BROADCAST_STATE__NEED_PTX_PowerOffMessage; // sync "power off"
                    }
                    LL_Helmet_ChangeStateTo_BeepBeforeOFF(); 
                }
                // sgulGeneralStep = ?  // no need set "sgulGeneralStep" since it has been set in the "LL_Helmet_ChangeStateTo_" above
            }
            break;
    }
    if( gtSysState.eModeOfWarningLight != gtPara.eModeOfWarningLight) { // if mode changed by bradcast
        gtSysState.eModeOfWarningLight  = gtPara.eModeOfWarningLight; // change the mode 
        { // do similar as "LL_Helmet_ChangeStateTo_NextMode"
            gtSysState.eBrakeState  = BRAKE_OFF;
            LL_HelmetActionWhenStateChanged();
            sgulGeneralStep = 0; sgulGeneralCnt = gulTimerCnt1ms; 
        }            
    }
    if(1 == needSwitchMode) { // mode switching when timeout for the 2nd click of double click
        if(DOUBLE_CLICK_TIMEOUT < LL_Timer_Elapsed_ms(sgulTimerCntPrevRelease)) { // if no 2nd release in 500ms            
            // switch flash mode.
            
            LL_Helmet_ChangeStateTo_NextMode();        
            TxCurrentFlashModeToApp();//TxCurrentFlashModeToApp();
            //LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_TurningBraking);            
            sgulBeepCnt = gulTimerCnt1ms;     
            needSwitchMode = 2;}//need buzzer 
    }else if(2 == needSwitchMode){
        if(       50 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { LL_LED_ON(  E_LL_PWM_BUZZER ); }
        else if(  100 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { LL_LED_OFF( E_LL_PWM_BUZZER ); needSwitchMode = 0;}
    }

    if(glAdapterBoardConnState == ADAPTEBOARD_DISCONNECT){ LL_Helmet_ChangeStateTo_BeepBeforeOFF(); }
}

//void LL_Helmet_ChangeStateTo_BeepBeforePairing(void)
//{
//    gtSysState.eOnOFF = SYS_BEEP_BEFORE_PAIRING;
//    sgulBeepCnt = gulTimerCnt1ms;    
//}

void LL_Helmet_ChangeStateTo_Pairing(void)
{
    gtSysState.eOnOFF = SYS_PAIRING;//SYS_ON;
    gtSysState.ulNoConnectionSinceSysOn = 1;
    gtSysState.eTurnState            = TURNING_NONE;
    gtSysState.eBrakeState           = BRAKE_OFF;
    LL_HelmetActionWhenStateChanged();
    
    LL_BLE_SetAdManu(LL_BLE_ADV_PAIRING_FLAG);
    battery_level_update();
    //ble_advertising_start(BLE_ADV_MODE_FAST); timeslot_sd_start();

    LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_BREATHING_MODE);
    LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_BREATHING_MODE); 

    //LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Pairing_APP);    
    
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms;    
}
//static unsigned long sgulBreathingBrightness = 0;
//static unsigned long sgulBreathingCnt = 0;
static unsigned long sgulBrakeOnOffClickCnt = 0;
static unsigned long sgulBrakeOnOffTimeCnt = 0;
//void IndicatorOfBrakeFuncOnOff__Start(void);
static inline void LL_Helmet_State_Pairing(void)
{
   
    T_LL_KEY_EVT tKeyEvt;
    switch(sgulGeneralStep) {
        case  0:
            // if ON/OFF key pressed: check whether a short or long press.
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
                sgulGeneralCnt = gulTimerCnt1ms;
                sgulGeneralStep++; 
            }
            break;    
        case  1:
            // if ON/OFF key released: exit pairing mode.
            if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) {
//                  LL_Helmet_ChangeStateTo_ON();
                    // 5 clicks to switch brake on/off:
                    sgulBrakeOnOffClickCnt++;
                    if(1 == sgulBrakeOnOffClickCnt) { 
                        // first click: start timing.
                        sgulBrakeOnOffTimeCnt = gulTimerCnt1ms;  
                    } else {
                        // following clicks: re-counting if timeout
                        if(5000 < LL_Timer_Elapsed_ms(sgulBrakeOnOffTimeCnt)) {
                            sgulBrakeOnOffClickCnt = 0;
                        }
                    }
                    if(5 <= sgulBrakeOnOffClickCnt) { sgulBrakeOnOffClickCnt = 0;
                        if(0 == gtPara.ulBrakeFunction) { 
                            gtPara.ulBrakeFunction = 1; gulFlashStoreNeeded = 1;          
                        } else { 
                            gtPara.ulBrakeFunction = 0; gulFlashStoreNeeded = 1;                        
                        }
                        //IndicatorOfBrakeFuncOnOff__Start();
                    }
                    // BACK TO STEP 0
                    sgulGeneralStep = 0;
            } else {
                    // if long pressed: OFF.
                    if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                        LL_Helmet_ChangeStateTo_BeepBeforeOFF(); 
                    }
            }
            break;    
    }    
    
    if(glAdapterBoardConnState == ADAPTEBOARD_DISCONNECT){ LL_Helmet_ChangeStateTo_BeepBeforeOFF(); }
}



void LL_Helmet_ChangeStateTo_BeepBeforeOFF(void)
{
    gtSysState.eOnOFF = SYS_BEEP_BEFORE_OFF;
    sgulBeepCnt = gulTimerCnt1ms; //LL_LED_ON(E_LL_PWM_BUZZER);
    //LL_LEDs_PowerOff_BeforeSleep();
//    LL_BatteryIndicatorOfPowerOn_Start(); // also need Low Battery Indicator  
    LL_BeepForTurning__OFF();
}
static inline void LL_Helmet_State_BeepBeforeOFF(void)
{  
    glPowerOnAnimationPlayOff = false;
    gtPanelPara.gulWs2812FrameDisplayCnt = 0;
    LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_POWEROFF);
    LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_POWEROFF);    
    
    if(       125 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { if(E_BEEP_MODE_OF_POWER_ON_OFF__BEEP == gtPara.beep_mode_of_power_on_off) { LL_LED_ON(  E_LL_PWM_BUZZER ); } }
    else if(  250 > LL_Timer_Elapsed_ms(sgulBeepCnt) ) { LL_LED_OFF( E_LL_PWM_BUZZER ); }
    else { 
//      if(LL_BatteryIndicatorOfPowerOn_IsFinished()) { // if the Low Battery Indicator done 
        if(true) {
            //LL_LEDs_PowerOff_BeforeSleep();             
            LL_Helmet_ChangeStateTo_OFF(); 
        } else { /* else, need wait for the Low Battery Indicator done */ }
    }
}


void LL_Helmet_ChangeStateTo_OFF(void)
{
    gtSysState.eOnOFF = SYS_OFF;
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms; 
    gulFlashStoreNeeded = 1;//in order to store battery.
 
}
static inline void LL_Helmet_State_OFF(void)
{
    char string[16];
    unsigned short sendatalen;
    switch(sgulGeneralStep) {
        case  0:
            sprintf(string, "DSC");
            sendatalen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &sendatalen, m_conn_handle);
            sgulGeneralCnt = gulTimerCnt1ms;    
            sgulGeneralStep++;
            break;
        case  1: if(500 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { sgulGeneralStep++; }
            break;
        case  2:
            sprintf(string, "BL:%u\0", LL_Battery_Level()); sendatalen = strlen(string);
            sendatalen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &sendatalen, m_conn_handle);
            sgulGeneralCnt = gulTimerCnt1ms;    
            sgulGeneralStep++;
            break;
        case  3: if(500 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { sgulGeneralStep++; }
            break;
        case  4: 
            if(BLE_CONN_HANDLE_INVALID != m_conn_handle) { 
                sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            }
            sgulGeneralCnt = gulTimerCnt1ms;    
            sgulGeneralStep++;
            break;
        case  5: if(2000 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) { sgulGeneralStep++; }
            break;
        case  6:
            if( 1 == LL_GPIO_InputRead(gatKeyCfg[LL_KEY_NUM_ONOFF].port, gatKeyCfg[LL_KEY_NUM_ONOFF].pin) ) { 
//                LL_Power_BeforeSleep(); 
                { //LL_Power_Sleep();
                    { // can save some but should not stop the adv
                        void LL_BLE_Adv_stop(void);
                        LL_BLE_Adv_stop();//uint32_t err_code = sd_ble_gap_adv_stop(); APP_ERROR_CHECK(err_code);
                        //(void) sd_ble_gap_scan_stop();
                    }
////                            { // save about 10mA
////                                timeslot_sd_stop(); 
////                            }
//                    { // app timer: save 0.13mA
//                        uint32_t app_timer_stop_all(void);
//                        app_timer_stop_all();
//                    }
//                    { // ADC  save about 0.41mA
//                      void LL_ADC_Stop(void);
//                      LL_ADC_Stop();
//                    }
                    while(1) {
                        //
                        sd_app_evt_wait(); // save about 3mA (from 13mA to 10mA)
                        //
                        if( gatKeyCfg[LL_KEY_NUM_ONOFF].normal != LL_GPIO_InputRead(gatKeyCfg[LL_KEY_NUM_ONOFF].port, gatKeyCfg[LL_KEY_NUM_ONOFF].pin) ) { // if key is pressed probably
                            wake_up_event__key_pressed = true;
                        }
                        //
                        if( wake_up_event__power_on_cmd_scanned
                        ||  wake_up_event__key_pressed          ) {
                            LL_Key_Init_With_No_Trigger(); // to avoid Nordic's "GPIOTE Latch" bug
                            break;
                        }
                    }
                    if(wake_up_event__key_pressed) { wake_up_event__key_pressed = false;
                        //LL_Power_BeforeWakeup(); 
                        gtSysState.eTurnState = TURNING_NONE; 

                        LL_Helmet_ChangeStateTo_Init(); // just like wake up from SYSTEM OFF
                    }
                    if(wake_up_event__power_on_cmd_scanned) { wake_up_event__power_on_cmd_scanned = false;
                        //LL_Power_BeforeWakeup(); 
                        gtSysState.eTurnState = TURNING_NONE; 
                        LL_Helmet_ChangeStateTo_BeepBeforeON();
                    }
                }
            } else {
                // do nothing, just wait key released.
            }
            break;
    }
}



// LL_BeepForTurning
unsigned long sgulBeepWhenTurning_Cnt = 0;
static inline void LL_BeepForTurning(void) { if(0 == sgulBeepWhenTurning_Cnt) return;
    unsigned long ulDelta = LL_Timer_Elapsed_ms(sgulBeepWhenTurning_Cnt);
    if((LL_LED_TURN_FLASH_ON>>1) > ulDelta) { LL_LED_ON(E_LL_PWM_BUZZER); } // >>1 : to make beep shorter
    else if((LL_LED_TURN_FLASH_ON + LL_LED_TURN_FLASH_OFF) > ulDelta) { LL_LED_OFF(E_LL_PWM_BUZZER); }
    else { LL_Timer_CntStop(sgulBeepWhenTurning_Cnt); }
}
void LL_BeepForTurning__OFF(void) {
    // module LL_LED
    LL_LED_OFF(E_LL_PWM_BUZZER);
    // module LL_BeepForTurning
    LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
}
static inline void LL_BeepForTurning__ON_forLR(void) {
    unsigned long ulON, ulOFF;
    switch(gtPara.eBeepMode) {
        case BEEP_WHEN_TURNING_ONOFF:
            // module LL_LED
            LL_LED_OFF(E_LL_PWM_BUZZER);
            // module LL_BeepForTurning
            LL_Timer_CntStart(sgulBeepWhenTurning_Cnt);
        break;
        case BEEP_EVERY_1_TURNING_FLASH:
            // module LL_LED
            ulON  = LL_LED_TURN_FLASH_ON>>1; // >>1 : to make beep shorter
            ulOFF = LL_LED_TURN_FLASH_ON + LL_LED_TURN_FLASH_OFF - ulON;
            LL_LED_Flashing(E_LL_PWM_BUZZER, ulON, ulOFF);
            // module LL_BeepForTurning
            LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
        break;
        case BEEP_EVERY_4_TURNING_FLASH:
            // module LL_LED
            ulON  = LL_LED_TURN_FLASH_ON>>1; // >>1 : to make beep shorter
            ulOFF = ((LL_LED_TURN_FLASH_ON + LL_LED_TURN_FLASH_OFF)<<2)  - ulON; // <<2: *4
            LL_LED_Flashing(E_LL_PWM_BUZZER, ulON, ulOFF);
            // module LL_BeepForTurning
            LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
        break;
        case BEEP_EVERY_8_TURNING_FLASH:
            // module LL_LED
            ulON  = LL_LED_TURN_FLASH_ON>>1; // >>1 : to make beep shorter
            ulOFF = ((LL_LED_TURN_FLASH_ON + LL_LED_TURN_FLASH_OFF)<<3)  - ulON; // <<3: *8
            LL_LED_Flashing(E_LL_PWM_BUZZER, ulON, ulOFF);
            // module LL_BeepForTurning
            LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
        break;
        case BEEP_NONE:
        default:
            // module LL_LED
            LL_LED_OFF(E_LL_PWM_BUZZER);
            // module LL_BeepForTurning
            LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
        break;
    }
}
static inline void LL_BeepForTurning__ON_forNONE(void) {
    switch(gtPara.eBeepMode) {
        case BEEP_WHEN_TURNING_ONOFF:
            // module LL_LED
            LL_LED_OFF(E_LL_PWM_BUZZER);
            // module LL_BeepForTurning
            LL_Timer_CntStart(sgulBeepWhenTurning_Cnt);
        break;
        default:
            // module LL_LED
            LL_LED_OFF(E_LL_PWM_BUZZER);
            // module LL_BeepForTurning
            LL_Timer_CntStop(sgulBeepWhenTurning_Cnt);
        break;
    }
}
void LL_BeepForTurning__ON(void) {
        switch(gtSysState.eTurnState) {
            case TURNING_L:     LL_BeepForTurning__ON_forLR();
            break;
            case TURNING_R:     LL_BeepForTurning__ON_forLR();
            break;
            case TURNING_NONE:  LL_BeepForTurning__ON_forNONE();
            break;
        }
}

static unsigned long ulCntTurnOffBrake = 0;
static inline void LL_Helmet_TurnOffBrakeLightAuto(void)
{
    if(BRAKE_OFF == gtSysState.eBrakeState) {
        ulCntTurnOffBrake = gulTimerCnt1ms; 
    } else if(2*1000 < LL_Timer_Elapsed_ms(ulCntTurnOffBrake)) { 
        gtSysState.eBrakeState = BRAKE_OFF;
        LL_HelmetActionWhenStateChanged();
    }
}
void LL_Helmet_TurnOffBrakeLightAuto_ReclockWhenReceiveNewBrake(void) {
    ulCntTurnOffBrake = gulTimerCnt1ms;     
}

static unsigned long ulCntTurnOffTurning = 0;
static inline void LL_Helmet_TurnOffTurningLightAuto(void)
{
    if(TURNING_NONE == gtSysState.eTurnState) {
        ulCntTurnOffTurning = gulTimerCnt1ms; 
    } else if(2*60*1000 < LL_Timer_Elapsed_ms(ulCntTurnOffTurning)) { 
        gtSysState.eTurnState = TURNING_NONE;
        LL_HelmetActionWhenStateChanged();
    }    
}

#ifndef _LL_DELETE_PRODUCTION_CODE
unsigned long gulCmdTestingMode = 0;
static unsigned char sgaucFinalTestPack[20];
void LL_Helmet_ChangeStateTo_ProductionTestingMode(void)
{
    gtSysState.eOnOFF = SYS_PRODUCTION_TESTING_MODE;
       
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_Y__LED_POWER_ENABLE);
 //   ble_advertising_start(BLE_ADV_MODE_FAST); timeslot_sd_start();
//    LL_Helmet_RedGreenFlashWhenTestMode_ON();
    gulCmdTestingMode = 0;
    // general
    sgulGeneralStep = 0;
    sgulGeneralCnt = gulTimerCnt1ms;    
}
static inline void LL_Helmet_State_ProductionTestingMode(void)
{
    T_LL_KEY_EVT tKeyEvt;
    unsigned long battery_level;
    uint16_t SendDataLen;
    switch(sgulGeneralStep) {
        case  0:
						LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_PRODUCT_TESTING);
						LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_PRODUCT_TESTING);
            // next step:
            sgulGeneralCnt = gulTimerCnt1ms;
            sgulGeneralStep++; 
            break;
        case 1:
            if( 500 < LL_Timer_Elapsed_ms(sgulGeneralCnt) ) {
                // next step:
                sgulGeneralCnt = gulTimerCnt1ms;
                sgulGeneralStep++; 
            }
            break;
        case 2:
            battery_level = LL_Battery_Level();
            if(LL_BATTERY_LEVEL_NONE == battery_level) break;
            // pack
            memcpy(&(sgaucFinalTestPack[0]), gtHwID.auc, 8);
            sgaucFinalTestPack[8] = LL_VER_x;
            sgaucFinalTestPack[9] = LL_VER_y;
						sgaucFinalTestPack[10] = LL_VER_z;
            memcpy(&(sgaucFinalTestPack[11]), gtPara.tRemote.HardwareID[gtPara.tRemote.ulIng].auc, 8);
            sgaucFinalTestPack[16] = (unsigned char)battery_level;
            // next step:
            sgulGeneralCnt = gulTimerCnt1ms;
            sgulGeneralStep++; 
            break;        
        case 3:
            SendDataLen = 17;
            if( NRF_SUCCESS != ble_nus_data_send(&m_nus, (uint8_t *)sgaucFinalTestPack, &SendDataLen, m_conn_handle) ) break;
            // next step:
            sgulGeneralCnt = gulTimerCnt1ms;
            sgulGeneralStep++; 
            break;
        case 4:
            // next step:
            sgulGeneralCnt = gulTimerCnt1ms;
            sgulGeneralStep++; 
            break;
    }
    
    // power off & LED-loop:
    if(0 == sgulXxxTestPowerOffStep) {
        // if ON/OFF key pressed: check whether a short or long press.
        if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
            sgulXxxTestPowerOffCnt = gulTimerCnt1ms;
            sgulXxxTestPowerOffStep++; 
        }
    } else if(1 == sgulXxxTestPowerOffStep) {
        // if ON/OFF key released: switch flash mode.
        if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) {
                sgulXxxTestPowerOffStep = 0;
        } else {
                // if long pressed: OFF.
                if(2000 < LL_Timer_Elapsed_ms(sgulXxxTestPowerOffCnt) ) { LL_Key_IgnoreNextRelease(LL_KEY_NUM_ONOFF); 
                    sgulXxxTestPowerOffStep = 0;
                    LL_Helmet_ChangeStateTo_BeepBeforeOFF(); 
                }
        }
    }    
}
#endif


void LL_Helmet_Mainloop(void)
{
    switch(gtSysState.eOnOFF) {
        case SYS_INIT                       : LL_Helmet_State_Init();                   break;
        case SYS_PCB_TESTING_MODE           : LL_Helmet_State_PcbTestingMode();         break;
        case SYS_PRODUCTION_TESTING_MODE    : LL_Helmet_State_ProductionTestingMode();  break;
        case SYS_BEEP_BEFORE_ON             : LL_Helmet_State_BeepBeforeON();           break;
        case SYS_ON                         : LL_Helmet_State_ON();                     break;
//        case SYS_BEEP_BEFORE_PAIRING        : LL_Helmet_State_BeepBeforePairing();      break;
        case SYS_PAIRING                    : LL_Helmet_State_Pairing();                break;
        case SYS_BEEP_BEFORE_OFF            : LL_Helmet_State_BeepBeforeOFF();          break;
        case SYS_OFF                        : LL_Helmet_State_OFF();                    break;
    }
    
    //LL_BLE_Broadcaster();
    
    LL_BeepForTurning();
    
    LL_Helmet_TurnOffBrakeLightAuto();   // turn off the brake LED after 2s:    
    LL_Helmet_TurnOffTurningLightAuto(); // turn off the turning LED after 2min:
}

