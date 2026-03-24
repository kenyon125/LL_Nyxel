/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <string.h>
#include "nrf_esb.h"

#include "LL_Hardware_cfg.h"
#include "LL_Power.h"
#include "LL_SysMode.h"
#include "LL_SysMode.h"
//#include "LL_Pairing.h"
#include "LL_Clock.h"
#include "LL_Timer.h"
#include "LL_PWM.h"
#include "LL_ESB.h"
#include "LL_Flash.h"
#include "LL_Para.h"
#include "LL_LED_Helmet.h"
#include "LL_HardwareID.h"
#include "LL_MSG.h"
extern void LL_Helmet_TurnOffBrakeLightAuto_ReclockWhenReceiveNewBrake(void);
#include "LL_Sync.h"
#include "LL_BLE_Broadcaster.h"



unsigned long gulBatteryOfRemote = 0xFFFFFFFF;
unsigned long canThisRemoteChangeSleepTime = 0;
unsigned long option_SleepTime_of_remote_from_app = LL_DEFAULT_OPTION_SLEEPTIME_OF_REMOTE;
void LL_HelmetActionWhenStateChanged(void) {    
    

    // turning light & buzzer
    if(gtSysState.eTurnState != gtSysState_prev.eTurnState) { 
         switch(gtSysState.eTurnState) {
            case TURNING_L:     
                if(BEEP_NONE != gtPara.eBeepMode) {        
                    LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_PWM_BUZZER, &(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]));   
                }  
                break;
            case TURNING_R:
                if(BEEP_NONE != gtPara.eBeepMode) {        
                    LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_PWM_BUZZER, &(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]));
                }
                break;
            case TURNING_NONE:
                if(BEEP_NONE != gtPara.eBeepMode) {
                    if(TURNING_L == gtSysState_prev.eTurnState){
                        LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_PWM_BUZZER, &(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]));
                    }
                    if(TURNING_R == gtSysState_prev.eTurnState){
                        LL_LED_FlashingByCustomer_SpecialForBuzzer(E_LL_PWM_BUZZER, &(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]));
                    }
                }
                break;
         } 
         LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_TurningBraking);
     }     
    // LED panel
    if(TURNING_L == gtSysState.eTurnState) {
        //turn left
        LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_TURNLEFT_SIGNAL);
        LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_TURNLEFT_SIGNAL);  

        //LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_TURNLEFT_FLASH_SIGNAL); 
        //LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_TURNLEFT_FLASH_SIGNAL);            
    } else if(TURNING_R == gtSysState.eTurnState) {
        //turn right
        LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_TURNRIGHT_SIGNAL);
        LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_TURNRIGHT_SIGNAL); 

        //LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_TURNRIGHT_FLASH_SIGNAL); 
        //LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_TURNRIGHT_FLASH_SIGNAL);        
    } else if((BRAKE_ON == gtSysState.eBrakeState) && (1 == gtPara.ulBrakeFunction)) {
        unsigned long flashing_pattern = LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, E_LL_LED_REAR);
        if(0x00FFFFFF == flashing_pattern) { // if solid
            LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_BRAKE_SIGNAL_SOLID);
            LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_BRAKE_SIGNAL_SHARPFLASH);  
        }else{
            LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_BRAKE_SIGNAL_SOLID);
            LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_BRAKE_SIGNAL_SOLID);  
        }            
        //brake
        LL_Helmet_TurnOffBrakeLightAuto_ReclockWhenReceiveNewBrake();
    } else {
        //gtSysState.eModeOfWarningLight means the 'Now for the number of modes'. gtPara.ePatternOfFrontLight[x] means 'The pattern number stored in the x pattern'
            switch(gtSysState.eModeOfWarningLight) {            
                case E_MODE_OF_WARNING_LIGHT__Mode1:  
                    LL_Drv_Ws2812_SetFrontAnimation(gtPara.ePatternOfFrontLight[0]);
                    LL_Drv_Ws2812_SetRearAnimation(gtPara.ePatternOfRearLight[0]); 
                    break;
                case E_MODE_OF_WARNING_LIGHT__Mode2:
                    LL_Drv_Ws2812_SetFrontAnimation(gtPara.ePatternOfFrontLight[1]);
                    LL_Drv_Ws2812_SetRearAnimation(gtPara.ePatternOfRearLight[1]); 
                    break;
                case E_MODE_OF_WARNING_LIGHT__Mode3:
                    LL_Drv_Ws2812_SetFrontAnimation(gtPara.ePatternOfFrontLight[2]);
                    LL_Drv_Ws2812_SetRearAnimation(gtPara.ePatternOfRearLight[2]);    
                    break;
                default: // should not happen
                    //LL_LED_Panel_SetAnimation(LL_LED_PANEL_ANIMATION_Circles_RGB, E_LL_ANIMATION__RE_TRIGGER_TYPE__NOT_IF_SAME);        
                    break;
            }
    }
                       
    // bak the new state
    memcpy(&gtSysState_prev, &gtSysState, sizeof(T_SysState));
}

unsigned char gulTurnSignalFlag = 0;
unsigned long gulMsgHandleNeeded = 0;
unsigned long   gulRSSI = 0;
T_MSG_toHelmet  sgtMsg;
void LL_MsgHandle_Helmet(T_MSG_toHelmet *ptMsg)
{
//    int i;
    
    if(    (SYS_ON      == gtSysState.eOnOFF)
        || (SYS_PAIRING == gtSysState.eOnOFF) ) {
                //if( (0 == gtPara.ulBrakeFunction) && ('K' == ptMsg->cStateBrake) ) { return; } // ignore the brake if this function off.
                //else                                                               {         } // continue.
    } else {
                return;
    }
    
    // first, check whether handle this msg or just ignore.
    if(SYS_PAIRING == gtSysState.eOnOFF) { // if Pairing Mode
        // pairing mode: any msg can be handled.
        gtPara.tRemote.ulIng = gtPara.tRemote.ulRoundRobin;
        memcpy( &(gtPara.tRemote.HardwareID[gtPara.tRemote.ulRoundRobin]) , 
                &(ptMsg->tHardwareID)                                     ,
                sizeof(T_LL_HardwareID)                                   );
        if(ptMsg->cStateBrake == 'C') { gtPara.tRemote.bits_isCheapRemote |=  (0x1<<gtPara.tRemote.ulIng); } 
        else /* not Cheap Remote */   { gtPara.tRemote.bits_isCheapRemote &= ~(0x1<<gtPara.tRemote.ulIng); } 
        //
        // When Remote Lite is paired, helmet is kicked out of the ecosystem
        if(ptMsg->cStateBrake == 'C') {
            { // copy from "NETID RESET"
                extern T_LL_HardwareID gtHwID;
                memcpy(&(gtPara.netID), &(gtHwID.auc[0]), LL_ESB_BASE_ADDR_LEN); gtPara.netID_exist = 0; gulFlashStoreNeeded = 1;    
            }                
        }
        //
        // exit pairing mode at once:  //giPaired_Remote = 1;
        gtPara.tRemote.ulRoundRobin++; if(DEVICE_NUM_OF_XXX <= gtPara.tRemote.ulRoundRobin) {gtPara.tRemote.ulRoundRobin=0;}
        gulFlashStoreNeeded = 1;////flash_store();                            
        LL_Helmet_ChangeStateTo_ON();
    } else { // else, not Pairing Mode
        #ifdef _NOT_CHECK_REMOTE_ID_IN_TURNING_MESSAGE
           // not check
        #else
        {
            if (1 == gtSysState.ulNoConnectionSinceSysOn) {
                for(int i=0; i<DEVICE_NUM_OF_XXX; i++ ) {
                    if( LL_HardwareID_IsEqual( &(gtPara.tRemote.HardwareID[i]) , 
                                               &(ptMsg->tHardwareID          ) ) ) {
                                                   gtSysState.ulNoConnectionSinceSysOn = 0;
                                                   gtPara.tRemote.ulIng = i;
                                                   break;
                                               }
                }
                if (1 == gtSysState.ulNoConnectionSinceSysOn) return; // not found the match one.
            } else if( LL_HardwareID_IsEqual( &(gtPara.tRemote.HardwareID[gtPara.tRemote.ulIng]) , 
                                              &(ptMsg->tHardwareID                             ) ) )  { 
                // msg from paired-and-active device, so this msg can be handled.
            } else {
                // not paired yet, this msg should be ignored.
                return;
            }
        }
        #endif    
    }        
            
    // then, handle it.
    switch( ptMsg->cStateTurn) {
        case 'O' : gtSysState.eTurnState = TURNING_NONE;gulTurnSignalFlag = 0;
        break;
        case 'L' : gtSysState.eTurnState = TURNING_L;gulTurnSignalFlag = 1;
        break;
        case 'R' : gtSysState.eTurnState = TURNING_R;gulTurnSignalFlag = 1;
        break;
    }
    switch( ptMsg->cStateBrake) {    
        case 'N' : gtSysState.eBrakeState = BRAKE_OFF; 
        break;
        case 'K' : if(1 == gtPara.ulBrakeFunction) { gtSysState.eBrakeState = BRAKE_ON; }
        break;
    }
    if('C' == ptMsg->cStateBrake) { // if remote Lite
        gulBatteryOfRemote = 0xCCCCCCCC;
        if(0x0101 == ptMsg->usVerDate) {
            canThisRemoteChangeSleepTime = 1;
        } else {
            canThisRemoteChangeSleepTime = 0;
        }
    } else {
        gulBatteryOfRemote = ptMsg->cBattery;
        canThisRemoteChangeSleepTime = 0;
    }
    
    LL_HelmetActionWhenStateChanged();
}



static unsigned char sgucPublicMessageBuffer[NRF_ESB_MAX_PAYLOAD_LENGTH];
static unsigned long sgulPublicMessageLength = 0;
void LL_MSG_Handle__PublicMessage_saveWhenInInterrupt(unsigned char *pMessage, unsigned long length) { 
    if(sizeof(sgucPublicMessageBuffer) < length || gtSysState.eOnOFF == SYS_PAIRING) return; // exception: no enough buffer or systate in pairing
    memcpy(sgucPublicMessageBuffer, pMessage, length); sgulPublicMessageLength = length;
}
void LL_MSG_Handle__PublicMessage_parseWhenInMainloop(void) {
    if(0 == sgulPublicMessageLength) return; // likely

//    nrf_esb_flush_tx(); // now I'm the slave, not send the old sync message any more
    
    if(sgulPublicMessageLength == sizeof(T_BROADCAST_MSG_PUBLIC)) {
        T_BROADCAST_MSG_PUBLIC eBroadcastMsgPublic; memcpy(&eBroadcastMsgPublic, sgucPublicMessageBuffer, sizeof(T_BROADCAST_MSG_PUBLIC));
        // clock
        LL_Clock__OTHERS_LED_CLOCK_snapshot(eBroadcastMsgPublic.ulFlashCounter);//gulTimerCntDiffFromSyncMaster = eBroadcastMsgPublic.ulFlashCounter - NRF_RTC1->COUNTER;
        //
        #if 0 // mode
            gtPara.eMode = (E_FlashTypeOfBackLight) (eBroadcastMsg.mode); gulFlashStoreNeeded = 1;
        #else // pattern
            #if 0 // overwrite directly
                gtPara.customer_flashing_timeslot_states[gtPara.eMode][E_LL_PWM_FRONT_WHITE_L] = eBroadcastMsg.slotsState;
                gtPara.customer_flashing_timeslot_states[gtPara.eMode][E_LL_PWM_FRONT_WHITE_R] = eBroadcastMsg.slotsState;
                gulFlashStoreNeeded = 1;
            #else // temp, not overwrite
                unsigned long sender               = eBroadcastMsgPublic.message_type;
                unsigned long patternOfFrontOrDual = eBroadcastMsgPublic.slotsStateOfFrontOrDual;
                unsigned long patternOfBack        = eBroadcastMsgPublic.slotsStateOfBack;
                unsigned long patternNowFront = LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, E_LL_LED_FRONT);    // for "Don't follow"
                //
                if(E_SYNC_MSG__FROM_HELMET == sender) {
                    LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);
                    LL_Para__set_flashing_pattern_fromSync(patternOfBack,        gtPara.eModeOfWarningLight, E_LL_LED_REAR);
                }
                if(E_SYNC_MSG__FROM_FIREFLY_FRONT == sender) {
                    LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);
                    LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_REAR); 
                }
                if(E_SYNC_MSG__FROM_FIREFLY_REAR == sender) {
                    LL_Para__set_flashing_pattern_fromSync(patternOfBack, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);  // "Follow"
                    LL_Para__set_flashing_pattern_fromSync(patternOfBack, gtPara.eModeOfWarningLight, E_LL_LED_REAR); 
                }
            #endif
        #endif
        if(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight] <= LED_ANIMATION_FASTFLASH_MODE){ //common flash pattern
            LL_Drv_Ws2812_SetFrontAnimation(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight]);
            LL_Drv_Ws2812_SetRearAnimation(gtPara.ePatternOfRearLight[gtPara.eModeOfWarningLight]); 
        }
    } else if(sgulPublicMessageLength == sizeof(T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT)) { 
        T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT eBroadcastMsgPublicRetransmit; memcpy(&eBroadcastMsgPublicRetransmit, sgucPublicMessageBuffer, sizeof(T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT));
        // clock
        LL_Clock__OTHERS_LED_CLOCK_snapshot(eBroadcastMsgPublicRetransmit.ulFlashCounter);//gulTimerCntDiffFromSyncMaster = eBroadcastMsgPublic.ulFlashCounter - NRF_RTC1->COUNTER;
    }
    
sgulPublicMessageLength = 0; }



void LL_MSG_toRemote_pack(void) {
    T_MSG_toRemote gtMsgToRemote;
    // make pkt:
//  memcpy(&(gtMsgToRemote.tHardwareID.auc), &(sgtMsg.tHardwareID.auc), LL_HARDWARE_ID_LEN);
//  memcpy(&(gtMsgToRemote.tHardwareID.auc), gtHwID.auc,                LL_HARDWARE_ID_LEN);
    memcpy(&(gtMsgToRemote.tHardwareID.auc), gtPara.netID,              LL_ESB_BASE_ADDR_LEN);
    gtMsgToRemote.tHardwareID.Vender = sgtMsg.tHardwareID.Vender;
    gtMsgToRemote.cBrakeFuncON = gtPara.ulBrakeFunction;
    gtMsgToRemote.ssKxyz[0]    = gtPara.ssKxyz[0];
    gtMsgToRemote.ssKxyz[1]    = gtPara.ssKxyz[1];
    gtMsgToRemote.ssKxyz[2]    = gtPara.ssKxyz[2];

    if(1 == canThisRemoteChangeSleepTime) { // if support "Auto Sleep Time set by App"
        gtMsgToRemote.ssThreshhold = (option_SleepTime_of_remote_from_app<<8)
                                   + (option_SleepTime_of_remote_from_app<<0); // 2 replicated bytes
    } else { // else, not support
        gtMsgToRemote.ssThreshhold = gtPara.ssThreshhold;
    }

    //
    // write to buffer
    #ifdef _NO_ACK_TO_REMOTE_WHEN_NROMAL_MODE   // only ack when pairing mode 
        if(SYS_PAIRING == gtSysState.eOnOFF) { 
            LL_ESB_WriteTxBuf(&gtMsgToRemote, sizeof(gtMsgToRemote), 0, false);
        } else {
            LL_ESB_WriteTxBuf(&gtMsgToRemote, sizeof(gtMsgToRemote), 0, true);
        }
    #else    
        LL_ESB_WriteTxBuf((unsigned char *)(&gtMsgToRemote), sizeof(gtMsgToRemote), 0, false);
    #endif
}
