/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_SYNC_H
#define _LL_SYNC_H



#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "boards.h"
#include "nrf_log.h"

#include "LL_Hardware_cfg.h"
#include "LL_HardwareID.h"
#include "LL_Clock.h"
#include "LL_Timer.h"
#include "LL_LED_Helmet.h"
#include "LL_MSG.h"
#include "LL_MSG_Handle_Helmet.h"
#include "LL_Para.h"
#include "LL_SysMode.h"
#include "LL_Flash.h"
#include "LL_ESB.h"

extern unsigned long clock_from_ble_evt_dispatch; // record the clock as early as possible, so that it will be more closed to the "sender's clock in adv"
extern unsigned long last_time_of_syncing_or_being_synced; // record the last time of syncing or being syncing so that can delay a while for next syncing



extern unsigned long gulESB_NeedTx; // 1: TX all; 2: TX only clock



#ifdef _LL_2p4GHz_RELAY
    extern unsigned long flag_for_relay_remote_msg;
    extern unsigned long time_for_relay_remote_msg;
    typedef struct {
        unsigned long esb;
        unsigned long pipe0;
        unsigned long pipe0_remote_msg;
        unsigned long pipe0_sync_msg;
        unsigned long pipe0_sync_poweroff;
        unsigned long pipe0_noise;
        unsigned long pipe0_relay;
    } T_ESB_RECEIVE_COUNT; extern T_ESB_RECEIVE_COUNT tEsbReceiveCount;
#endif
    
    
    
typedef enum {
    E_BROADCAST_STATE__OFF = 0,
    E_BROADCAST_STATE__NEED_PTX,
    E_BROADCAST_STATE__NEED_PTX_OnlyClock,
    E_BROADCAST_STATE__NEED_PTX_PowerOffMessage,
    E_BROADCAST_STATE__PTX_STARTED,
    #if 0 // wait "TX done" before switching to PRX
        E_BROADCAST_STATE__WAIT_TX_DONE,
    #endif
    E_BROADCAST_STATE__WAIT_RESYNC,     // need re-sync since the clock is not accurate enough
    E_BROADCAST_STATE__RESYNC,          // need re-sync since the clock is not accurate enough
} E_BROADCAST_STATE;
extern E_BROADCAST_STATE geBroadcastState;

#if 0 // timing by 2.4GHz tick
#define BROADCAST_STATE__TIMEOUT_OF_WAIT_RESYNC     ((10*60*1000*1000)/LL_TIMESLOT_DISTANCE) // unit: tick number of 2.4GHz
#else // timing by software timer
#define BROADCAST_STATE__TIMEOUT_OF_WAIT_RESYNC     (10*60*1000)    // unit: ms
#endif
    
extern unsigned long gulBroadcastCnt;
extern unsigned long timestamp_befroe_waiting;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Messages
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    E_SYNC_MSG__BROADCAST_MSG = 0,                // not used yet, only checked by length (32) in pipe 1
    //
    E_SYNC_MSG__BROADCAST_MSG_PUBLIC,             // not used yet, only checked by length (20) in pipe 1
    E_SYNC_MSG__BROADCAST_MSG_PUBLIC_RE_TRANSMIT, // not used yet, only checked by length ( 8) in pipe 1
    //
    E_SYNC_MSG__BROADCAST_MSG_POWER_OFF,          // not used yet, only checked by length ( 4) in pipe 1
    //
    E_SYNC_MSG__REMOTE_MSG_RELAY, // not controlled by "#ifdef _LL_2p4GHz_RELAY" since it should be always hold this enum place (value) as a interface
    //
    // need distinguish the sender of T_BROADCAST_MSG_PUBLIC acc to the Sync Scenario: https://docs.google.com/spreadsheets/d/1LIPo5awQxwPrzHrPcGvLcwq7ZkRubCufG0SOpG5p5VI/edit?pli=1#gid=1013373883
    E_SYNC_MSG__FROM_HELMET,
    E_SYNC_MSG__FROM_FIREFLY_FRONT,
    E_SYNC_MSG__FROM_FIREFLY_REAR,
    E_SYNC_MSG__FROM_FIREFLY_LR_ONLY,
} E_SYNC_MSG;



typedef struct {
    unsigned long ulFlashCounter;

//    unsigned long  ulFlashMode;
////    unsigned short usFlashDurationON;
////    unsigned short usFlashDurationOFF;
    unsigned long  slotsState;
    unsigned long  mode;//unsigned char  selfID[4];
    T_LL_HardwareID pairedRemoteID[2];
} T_BROADCAST_MSG;

typedef struct {
    unsigned long message_type;
    unsigned long ulFlashCounter;
    unsigned long mode;
    unsigned long slotsStateOfFrontOrDual;  // it's Dual Red actually when Profile is Rear
    unsigned long slotsStateOfBack;         // no use when Profile is Front
} T_BROADCAST_MSG_PUBLIC;
//
typedef struct {
    unsigned long message_type;
    unsigned long ulFlashCounter;
} T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT;

typedef struct {
    unsigned long message_type;
} T_BROADCAST_MSG_POWER_OFF;



void LL_Sync(void);



#endif
