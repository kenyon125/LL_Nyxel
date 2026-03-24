/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Sync.h"



unsigned long clock_from_ble_evt_dispatch; // record the clock as early as possible, so that it will be more closed to the "sender's clock in adv"
unsigned long last_time_of_syncing_or_being_synced = 0xFFFFFFFF; // record the last time of syncing or being syncing so that can delay a while for next syncing, 0xFFFFFFFF can make a "timeout" when user check it Power ON (while the gulTimerCnt is 0) 



#define ESB_PIPE_PRIVATE 0 // means: shared with remotes
#define ESB_PIPE_PUBLIC  1
#define ESB_PIPE_ALTERNATELY    0xFF    // IF_PUBLIC_ENABLED
//
#define ESB_RETRANSMISSION_TIMES    15
#define ESB_RETRANSMISSION_INTERVAL 500 // unit: us



//static void sync_Clock_FrontPattern_PairedRemotes(unsigned long pipe) {
//    unsigned long ulFlashCounter = LL_Clock__LED_CLOCK_get();
//    //unsigned long mode = gtSysState.eFlashTypeOfBackLight;
//    //
//    T_BROADCAST_MSG gtBroadcastMsg;
//    // clock
//    gtBroadcastMsg.ulFlashCounter = ulFlashCounter;
//    // both mode and flashing
//    //gtBroadcastMsg.mode = mode;
//    //gtBroadcastMsg.slotsState = LL_Para__get_flashing_pattern(mode, E_LL_LED_FrontWhiteL);//E_LL_LED_FRONT
//    // paired remote
//    memcpy( &(gtBroadcastMsg.pairedRemoteID[0]), &(gtPara.tRemote.HardwareID[0]), sizeof(T_LL_HardwareID) );    
//    memcpy( &(gtBroadcastMsg.pairedRemoteID[1]), &(gtPara.tRemote.HardwareID[1]), sizeof(T_LL_HardwareID) );    
//    //
//    LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsg), sizeof(gtBroadcastMsg), pipe, true);
//    //
//    // now I'm the sync master, so no need to consider the "diff"
//    LL_CLOCK__clearTheSnapshotAsMaster();            
//}
//
static void sync_Clock_FrontRearPattern(unsigned long pipe) {
    unsigned long ulFlashCounter = LL_Clock__LED_CLOCK_get();
    unsigned long mode = gtSysState.eModeOfWarningLight;
    //
    T_BROADCAST_MSG_PUBLIC gtBroadcastMsgPublic;
    gtBroadcastMsgPublic.message_type            = E_SYNC_MSG__FROM_HELMET;
    gtBroadcastMsgPublic.ulFlashCounter          = ulFlashCounter;
    gtBroadcastMsgPublic.mode                    = mode;
    gtBroadcastMsgPublic.slotsStateOfFrontOrDual = LL_Para__get_flashing_pattern(mode, E_LL_LED_FRONT);//E_LL_LED_FRONT
    gtBroadcastMsgPublic.slotsStateOfBack        = LL_Para__get_flashing_pattern(mode, E_LL_LED_REAR);
    //
    for(int i = 0; i < ESB_RETRANSMISSION_TIMES; i++) { nrf_delay_us(ESB_RETRANSMISSION_INTERVAL);
        if(ESB_PIPE_ALTERNATELY == pipe) {
            if(1) { // always
                LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublic), sizeof(gtBroadcastMsgPublic), ESB_PIPE_PRIVATE, true);
            }
            if(1 == gtPara.publicNetworkEnabled) { // if public sync is enabled
                LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublic), sizeof(gtBroadcastMsgPublic), ESB_PIPE_PUBLIC,  true);                
            }
        } else {
            LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublic), sizeof(gtBroadcastMsgPublic), pipe, true);
        }
    }
    //
    // now I'm the sync master, so no need to consider the "diff"
    LL_CLOCK__clearTheSnapshotAsMaster();            
}    
//
static void sync_Clock(unsigned long pipe) {
    unsigned long ulFlashCounter = LL_Clock__LED_CLOCK_get();
    //
    T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT gtBroadcastMsgPublicRetransmit;
    gtBroadcastMsgPublicRetransmit.ulFlashCounter = ulFlashCounter;
    //
    for(int i = 0; i < ESB_RETRANSMISSION_TIMES; i++) { nrf_delay_us(ESB_RETRANSMISSION_INTERVAL);
        if(ESB_PIPE_ALTERNATELY == pipe) {
            if(1) { // always
                LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublicRetransmit), sizeof(gtBroadcastMsgPublicRetransmit), ESB_PIPE_PRIVATE, true);
            }
            if(1 == gtPara.publicNetworkEnabled) { // if public sync is enabled
                LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublicRetransmit), sizeof(gtBroadcastMsgPublicRetransmit), ESB_PIPE_PUBLIC,  true);
            }                    
        } else {
            LL_ESB_WriteTxBuf((unsigned char *)(&gtBroadcastMsgPublicRetransmit), sizeof(gtBroadcastMsgPublicRetransmit), pipe, true);
        }
    }
    //
    // now I'm the sync master, so no need to consider the "diff"
    LL_CLOCK__clearTheSnapshotAsMaster();            
}    
//
static void sync_PowerOff(unsigned long pipe) {
    unsigned long ulFlashCounter = LL_Clock__LED_CLOCK_get();
    //
    T_BROADCAST_MSG_POWER_OFF msg;
    msg.message_type = 0;
    //
		for(int i = 0; i < ESB_RETRANSMISSION_TIMES; i++) { nrf_delay_us(ESB_RETRANSMISSION_INTERVAL);
				LL_ESB_WriteTxBuf((unsigned char *)(&msg), sizeof(msg), pipe, true);
		}
//    //
//    // now I'm the sync master, so no need to consider the "diff"
//    LL_CLOCK__clearTheSnapshotAsMaster();            
}    
//
#ifdef _LL_2p4GHz_RELAY
static void sync_RemoteMsg(unsigned long pipe) {
    #if 0 // Firefly: a different message
    {
        T_BROADCAST_MSG__RELAY_OF_REMOTE_MSG msg;
    //  msg.message_type = E_SYNC_MSG__REMOTE_MSG_RELAY;
        memcpy(&(msg.tMsgToHelmet), &sgtMsg, sizeof(T_MSG_toHelmet));
        //
        #if 0 // send once
            LL_ESB_WriteTxBuf((unsigned char *)(&msg), sizeof(msg), pipe, true);
        #else // send multiple times
            for(int i = 0; i < ESB_RETRANSMISSION_TIMES; i++) { nrf_delay_us(ESB_RETRANSMISSION_INTERVAL);
                LL_LED_ON(E_LL_LED_PCB_B); // blue light indicator for retransmit
                LL_ESB_WriteTxBuf((unsigned char *)(&msg), sizeof(msg), pipe, true); //nrf_delay_us(600);
            }
        #endif
    }
    #else // Ultra: same as Remote's message
    {
        for(int i = 0; i < ESB_RETRANSMISSION_TIMES; i++) { nrf_delay_us(ESB_RETRANSMISSION_INTERVAL);
            //LL_LED_ON(E_LL_LED_PCB_B); // blue light indicator for retransmit
            LL_ESB_WriteTxBuf((unsigned char *)(&sgtMsg), sizeof(sgtMsg), pipe, true); //nrf_delay_us(600);
        }        
    }
    #endif
}
#endif



#ifdef _LL_2p4GHz_RELAY
    unsigned long flag_for_relay_remote_msg = 0;
    unsigned long time_for_relay_remote_msg = 0;
//    T_ESB_RECEIVE_COUNT tEsbReceiveCount = {
//        .esb = 0,
//        .pipe0 = 0,
//        .pipe0_remote_msg = 0,
//        .pipe0_sync_msg = 0,
//        .pipe0_sync_poweroff = 0,
//        .pipe0_noise = 0,
//        .pipe0_relay = 0.
//    };
#endif
unsigned long gulflag = 0;
E_BROADCAST_STATE geBroadcastState = E_BROADCAST_STATE__OFF;
unsigned long gulBroadcastCnt = 0;
unsigned long timestamp_befroe_waiting;
void LL_Sync(void) {
//    uint32_t err_code = NRF_SUCCESS; // as default
    #ifdef _LL_2p4GHz_RELAY
//        LL_LED_OFF(E_LL_LED_PCB_B); // blue light indicator for retransmit
        if(flag_for_relay_remote_msg) {
            // Why delay 100ms here:
            //      Remote nRF51 / nRF52 / Lite: 15 (maximum) times retransmit with 6ms interval, 90ms totally.
            //      Remote USB-C: 4 times retransmit with 6 ms interval, 24ms totally.
            //      So 100ms should be enough to wait for Remote's sending done.
            if(LL_Timer_Elapsed_ms(time_for_relay_remote_msg) >= 100) { time_for_relay_remote_msg = gulTimerCnt1ms;
                bool selective_auto_ack = true; esb_start_PTX(selective_auto_ack);
                sync_RemoteMsg(ESB_PIPE_PRIVATE);      
            }                
        flag_for_relay_remote_msg = 0; return; }
    #endif
        
    switch(geBroadcastState) {
        case E_BROADCAST_STATE__OFF: {
            {
                bool selective_auto_ack = (gtSysState.eOnOFF == SYS_PAIRING) ? false : true; esb_start_PRX(selective_auto_ack); 
                LL_MSG_toRemote_pack();
            }
        break; }
        case E_BROADCAST_STATE__NEED_PTX: {
            {
                bool selective_auto_ack = true; esb_start_PTX(selective_auto_ack);
                //
                {
                    sync_Clock_FrontRearPattern(ESB_PIPE_ALTERNATELY);
                }
                //
                // now I'm the sync master, so no need to consider the "diff"
                LL_CLOCK__clearTheSnapshotAsMaster();        
            }
            //
            geBroadcastState = E_BROADCAST_STATE__PTX_STARTED;
            gulBroadcastCnt = 0;
        break; }
        case E_BROADCAST_STATE__NEED_PTX_OnlyClock: {
            {
                bool selective_auto_ack = true; esb_start_PTX(selective_auto_ack);
                //
                {
                    sync_Clock(ESB_PIPE_ALTERNATELY);
                }
                //
                // now I'm the sync master, so no need to consider the "diff"
                LL_CLOCK__clearTheSnapshotAsMaster();        
            }
            geBroadcastState = E_BROADCAST_STATE__PTX_STARTED;
            gulBroadcastCnt = 0;            
        break; }
        case E_BROADCAST_STATE__NEED_PTX_PowerOffMessage: {
            {
                bool selective_auto_ack = true; esb_start_PTX(selective_auto_ack);
                //
//                sync_Clock_FrontPattern_PairedRemotes(ESB_PIPE_PRIVATE);
//                sync_Clock(ESB_PIPE_PUBLIC);//if(2 == gulESB_NeedTx)
                sync_PowerOff(ESB_PIPE_PRIVATE);
//                //
//                // now I'm the sync master, so no need to consider the "diff"
//                LL_CLOCK__clearTheSnapshotAsMaster();        
            }
            geBroadcastState = E_BROADCAST_STATE__PTX_STARTED;
            gulBroadcastCnt = 0;                        
        break; }
        case E_BROADCAST_STATE__PTX_STARTED: { gulBroadcastCnt++;
						gulflag++;
            ////unsigned long onlyClock = 0; err_code = esb_start_PTX(onlyClock);
            if(gulBroadcastCnt >= 3) { geBroadcastState = E_BROADCAST_STATE__WAIT_RESYNC; gulBroadcastCnt = 0; timestamp_befroe_waiting = gulTimerCnt1ms; }
        break; }
        case E_BROADCAST_STATE__WAIT_RESYNC: { gulBroadcastCnt++;
            {
                bool selective_auto_ack = (gtSysState.eOnOFF == SYS_PAIRING) ? false : true; esb_start_PRX(selective_auto_ack); 
                LL_MSG_toRemote_pack();
            }
            if(LL_Timer_Elapsed_ms(timestamp_befroe_waiting) >= BROADCAST_STATE__TIMEOUT_OF_WAIT_RESYNC) {
                geBroadcastState = E_BROADCAST_STATE__RESYNC; gulBroadcastCnt = 0; 
            }            
        break; }
        case E_BROADCAST_STATE__RESYNC: { gulBroadcastCnt++;
            {
                bool selective_auto_ack = true; esb_start_PTX(selective_auto_ack);
                //
                {
                    sync_Clock(ESB_PIPE_ALTERNATELY);
                }
                //
                // now I'm the sync master, so no need to consider the "diff"
                LL_CLOCK__clearTheSnapshotAsMaster();        
            }
            if(gulBroadcastCnt >= 1) { 
                geBroadcastState = E_BROADCAST_STATE__WAIT_RESYNC; timestamp_befroe_waiting = gulTimerCnt1ms;
                gulBroadcastCnt = 0; 
            }
        break; }
    }
//    APP_ERROR_CHECK(err_code);    
}
