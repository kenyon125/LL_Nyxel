/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdio.h>  // sprintf
#include <string.h>
#include "PCA10040.h"
#include "nrf_delay.h"
#include "app_error.h"

#include "ble_hci.h"
#include "ble_advertising.h"
void LL_BLE_SetAdManu(unsigned short manu);
void advertising_init(void);
void on_adv_evt(ble_adv_evt_t ble_adv_evt);
extern uint16_t m_conn_handle;
extern void battery_level_update(void);

#include "LL_Clock.h" // get clock for sync
#include "LL_Timer.h"
#include "LL_Para.h" // netID
#include "LL_SysMode.h"
#include "LL_Para.h"
#include "LL_Battery.h"
#include "LL_BatteryIndicator.h"
#include "LL_Battery_Charge.h"
#include "LL_MSG.h"
#include "LL_MSG_Handle_Helmet.h"
#include "LL_BLE.h" // BLE Company ID
#include "LL_Sync.h"
#include "LL_BLE_Broadcaster.h"
#define LL_BLE_ADV_PAIRING_FLAG 0x0101

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// acc to "Lumos Interface for Partners (BLE Advertising, V1.4)"
//
#define MSG_Pairing               0x01
#define MSG_Lumos_Lights_State    0x02
#define MSG_Battery               0x03
#define MSG_Sync_Clock            0x05
#define MSG_Power_ONOFF           0x06
#define MSG_TurningAndBraking     0x07
#define MSG_FlashingPattern       0x08
//
#define MSGSIZE_Pairing             10//11
#define MSGSIZE_Lumos_Lights_State  12
#define MSGSIZE_Battery             11
#define MSGSIZE_Sync_Clock          13
#define MSGSIZE_Power_ONOFF         11
#define MSGSIZE_TurningAndBraking   11
#define MSGSIZE_FlashingPattern     17
//
static void adv__Pairing(void);
//
#define LIGHT_ON__front_L       (1<<0)
#define LIGHT_ON__front_R       (1<<1)
#define LIGHT_ON__rear_L        (1<<2)
#define LIGHT_ON__rear_R        (1<<3)
#define LIGHT_ON__turning_L     (1<<4)
#define LIGHT_ON__turning_R     (1<<5)
#define LIGHT_ON__braking_L     (1<<6)
#define LIGHT_ON__braking_R     (1<<7)
#define LIGHT_ON__side_L        (1<<8)
#define LIGHT_ON__side_R        (1<<9)
static void adv__Lumos_Lights_State(unsigned long lights_state);
//
static void adv__Battery(unsigned long battery_level);
//
static void adv__Sync_Clock(void);
//
#define POWER_STATE__OFF    0x00
#define POWER_STATE__ON     0x01
static void adv__Power_ONOFF(unsigned char power_state);
//
#include "LL_SysMode.h" // E_TURN_STATE and E_BRAKE_STATE
#define TURNING_CMD_NONE      0
#define TURNING_CMD_L         1
#define TURNING_CMD_R         2
#define BRAKING_CMD_NONE      0
#define BRAKING_CMD_BRAKING   1
static void adv__TurningAndBraking(E_TURN_STATE eTurnState, E_BRAKE_STATE eBrakeState);
//
#include "LL_Sync.h" // E_SYNC_MSG
static void adv__FlashingPattern(unsigned char sender, unsigned long pattern_front, unsigned long pattern_rear);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1 // we need change the BLE name between "Ultra" (for Lumos APP) and "LMM" (for other Lumos devices)
#include "ble_advdata.h"
void update_device_name(const char *new_name) { // similar to "gap_params_init"
    uint32_t err_code;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)new_name, strlen(new_name));
    APP_ERROR_CHECK(err_code);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ble_advertising_init_t          init;
static ble_advdata_manuf_data_t advdata_manuf_data;
static unsigned char            spec_data[20];      // 20: max of Lumos BLE Advertisement

static void common_stop_init_start(unsigned long size_including_2_bytes_Company_ID, unsigned long adv_interval_ms, unsigned long adv_timeout_s) {
    uint32_t      err_code;

    {
        LL_BLE_Adv_stop();
    }
    { // BLE name
        ble_gap_conn_sec_mode_t sec_mode;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
        err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)"LMM", strlen("LMM"));
        APP_ERROR_CHECK(err_code);        
    }    
    { // copy from advertising_init()
        memset(&init, 0, sizeof(init)); {
            init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
            init.advdata.include_appearance = true;
            init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
            {
                advdata_manuf_data.company_identifier = LL_BLE_COMPANY_ID_OF_LUMOS; // LUMOS Company ID
                {
                    spec_data[1] = 0xCC; // Signature
                    spec_data[2] = gtPara.netID[0];
                    spec_data[3] = gtPara.netID[1];
                    spec_data[4] = gtPara.netID[2];
                    spec_data[5] = gtPara.netID[3];
                    spec_data[6] = 0x00; // Message Count
                    spec_data[7] = LL_BLE_Broadcaster__Device_Type;
                }
                advdata_manuf_data.data.size    = size_including_2_bytes_Company_ID - 2; // -2: the 2 bytes data "Company ID"
                advdata_manuf_data.data.p_data  = spec_data;
            }; init.advdata.p_manuf_specific_data = &advdata_manuf_data;
            {
                init.config.ble_adv_fast_enabled  = true;
                init.config.ble_adv_fast_interval = MSEC_TO_UNITS(adv_interval_ms, UNIT_0_625_MS); // adv_interval_ms: ms;
                init.config.ble_adv_fast_timeout  = adv_timeout_s; // adv_timeout_s: s;
            }
            {
                init.evt_handler = on_adv_evt;                
            }
        }
        {
            LL_BLE_Adv_init(&init);
        }
    }
    {   
        lumos_connectable = false;
        LL_BLE_Adv_start();        
        lumos_connectable = true; 
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void adv__Pairing(void) {
    spec_data[0] = MSG_Pairing;
//  spec_data[8] = 0x00;
    common_stop_init_start(MSGSIZE_Pairing, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Pairing);
}

static void adv__Lumos_Lights_State(unsigned long lights_state) {
    spec_data[0] = MSG_Lumos_Lights_State;
    spec_data[8] = lights_state;
    spec_data[9] = lights_state;
    common_stop_init_start(MSGSIZE_Lumos_Lights_State, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Lumos_Lights_State);
}

static void adv__Battery(unsigned long battery_level) {
    spec_data[0] = MSG_Battery;
    spec_data[8] = (unsigned char)battery_level;
    common_stop_init_start(MSGSIZE_Battery, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Battery);
}

static void adv__Sync_Clock(void) {
    unsigned long clock_now = LL_Clock__LED_CLOCK_get();
    spec_data[ 0] = MSG_Sync_Clock;
    spec_data[ 8] = (unsigned char)(clock_now >>  0);
    spec_data[ 9] = (unsigned char)(clock_now >>  8);
    spec_data[10] = (unsigned char)(clock_now >> 16);
    common_stop_init_start(MSGSIZE_Sync_Clock, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Sync_Clock);
}

static void adv__Power_ONOFF(unsigned char power_state) {
    spec_data[0] = MSG_Power_ONOFF;
    spec_data[8] = power_state;
    common_stop_init_start(MSGSIZE_Power_ONOFF, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_Power_ONOFF);
}

static void adv__TurningAndBraking(E_TURN_STATE eTurnState, E_BRAKE_STATE eBrakeState) {
    unsigned char turning_cmd; switch(eTurnState) {
        case TURNING_L:     turning_cmd = TURNING_CMD_L;    break;
        case TURNING_R:     turning_cmd = TURNING_CMD_R;    break;
        case TURNING_NONE:  turning_cmd = TURNING_CMD_NONE; break;
        default:            turning_cmd = TURNING_CMD_NONE; break; // should not happen
    }
    unsigned char braking_cmd; switch(eBrakeState) {
        case BRAKE_ON:  braking_cmd = BRAKING_CMD_BRAKING;  break;
        case BRAKE_OFF: braking_cmd = BRAKING_CMD_NONE;     break;
        default:        braking_cmd = BRAKING_CMD_NONE;     break; // should not happen
    }        
    spec_data[0] = MSG_TurningAndBraking;
    spec_data[8] = ((turning_cmd & 0x0F) << 4)
                 | ((braking_cmd & 0x0F) << 0);
    common_stop_init_start(MSGSIZE_TurningAndBraking, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_TurningAndBraking);
}

static void adv__FlashingPattern(unsigned char sender, unsigned long pattern_front, unsigned long pattern_rear) {
    spec_data[ 0] = MSG_FlashingPattern;
    spec_data[ 8] = sender;
    spec_data[ 9] = (pattern_front & 0x000000FF) >>  0;
    spec_data[10] = (pattern_front & 0x0000FF00) >>  8;
    spec_data[11] = (pattern_front & 0x00FF0000) >> 16;
    spec_data[12] = (pattern_rear & 0x000000FF) >>  0;
    spec_data[13] = (pattern_rear & 0x0000FF00) >>  8;
    spec_data[14] = (pattern_rear & 0x00FF0000) >> 16;
    common_stop_init_start(MSGSIZE_FlashingPattern, LL_BLE_Broadcaster__interval_and_timeout_of_MSG_FlashingPattern);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// functions provided by LL_BLE_Broadcaster
//
// common ones
#define FUNC__IDLE                              0
#define FUNC__DELAY_ms                          1
//#define FUNC__START_ANOTHER_THREAD            2
//
// old ones for Lumos APP
#define FUNC__START_ADV_FOR_APP_PAIRING         3
#define FUNC__START_ADV_FOR_APP_CONNECTING      4
//
// new ones for Lumos devices
#define FUNC__START_ADV_MSG_Pairing             5
#define FUNC__START_ADV_MSG_Lumos_Lights_State  6
#define FUNC__START_ADV_MSG_Battery             7
#define FUNC__START_ADV_MSG_Sync_Clock          8
#define FUNC__START_ADV_MSG_Power_ONOFF_ON      9
#define FUNC__START_ADV_MSG_Power_ONOFF_OFF     10
#define FUNC__START_ADV_MSG_TurningAndBraking   11
#define FUNC__START_ADV_MSG_FlashingPattern     12
//
// switch thread
#define FUNC__SWITCH_TO_THREAD__Pairing_APP             20
#define FUNC__SWITCH_TO_THREAD__Normal_Mode             21
#define FUNC__SWITCH_TO_THREAD__Normal_Mode_Sync_Clock  22



// user defined threads
const unsigned long thread__LL_BLE_Broadcaster__Idle[] = {
    FUNC__IDLE
};
const unsigned long thread__LL_BLE_Broadcaster__Pairing_APP[] = {
    FUNC__START_ADV_FOR_APP_PAIRING,
    FUNC__IDLE // no need switch to "devices pairing" until button event
};
const unsigned long thread__LL_BLE_Broadcaster__Pairing_Devices[] = { // trigger by button event
    FUNC__START_ADV_MSG_Pairing,
    FUNC__DELAY_ms, 5*1000, // 5s is enough for other devices to get the pairing message by scanning
    FUNC__SWITCH_TO_THREAD__Pairing_APP // back to pairing APP
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode[] = {
    FUNC__START_ADV_FOR_APP_CONNECTING,
    FUNC__DELAY_ms, 1*60*1000, // 1min for APP to connect to
    //
    FUNC__SWITCH_TO_THREAD__Normal_Mode_Sync_Clock // switch to Sync Clock
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Sync_Clock[] = {
    FUNC__START_ADV_MSG_Sync_Clock,
    FUNC__DELAY_ms, 130, // 130ms is easier got by other 100ms/200ms scanning 
    FUNC__START_ADV_MSG_Sync_Clock,
    FUNC__DELAY_ms, 130,
    FUNC__START_ADV_MSG_Sync_Clock,
    FUNC__DELAY_ms, 130,
    FUNC__START_ADV_MSG_Sync_Clock,
    FUNC__DELAY_ms, 130,
    FUNC__START_ADV_MSG_Sync_Clock,
    FUNC__DELAY_ms, 130,
    //
    FUNC__SWITCH_TO_THREAD__Normal_Mode // back to Normal Mode
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Sync_FlashingPattern[] = {
    FUNC__START_ADV_MSG_FlashingPattern,
    FUNC__DELAY_ms, 5*1000,
    FUNC__SWITCH_TO_THREAD__Normal_Mode_Sync_Clock // switch to Sync Clock
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_TurningBraking[] = {
    FUNC__START_ADV_MSG_Lumos_Lights_State, // for Cyber
    FUNC__DELAY_ms, 5*1000,
    FUNC__START_ADV_MSG_TurningAndBraking,
    FUNC__DELAY_ms, 5*1000,
    FUNC__SWITCH_TO_THREAD__Normal_Mode // back to Normal Mode
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Power_ON[] = {
    FUNC__START_ADV_MSG_Power_ONOFF_ON,
    FUNC__DELAY_ms, 5*1000,//10*1000,//20*1000,
    FUNC__SWITCH_TO_THREAD__Normal_Mode_Sync_Clock // switch to Sync Clock
};
const unsigned long thread__LL_BLE_Broadcaster__Normal_Mode_Power_OFF[] = {
    FUNC__START_ADV_MSG_Power_ONOFF_OFF,
    FUNC__DELAY_ms, 20*1000,
    FUNC__IDLE
};



static const unsigned long *thread = thread__LL_BLE_Broadcaster__Idle;
static unsigned long step = 0;
static unsigned long time = 0;

void LL_BLE_Broadcaster__start(const unsigned long *thread_xxx) { 
    thread = thread_xxx; 
    step = 0;
    time = gulTimerCnt1ms; 
}

void LL_BLE_Broadcaster(void) {
    unsigned long func = thread[step]; switch(func) {
        // common
        case FUNC__IDLE: {
            // nothing here
        }; break; // continue this step
        case FUNC__DELAY_ms: {
            unsigned long delay_ms = thread[step + 1];
            if(delay_ms <= LL_Timer_Elapsed_ms(time)) { step += 2; time = gulTimerCnt1ms; }; // if timeout, then go to next step
        }; break; // continue this step
//        case FUNC__START_ANOTHER_STEPS: {
//            steps = (FUNC *)(*(steps + 1));
//        }; break; // DO NOT MODIFY "steps" has been updated to another new steps
        //
        // old ones for Lumos APP
        case FUNC__START_ADV_FOR_APP_PAIRING: {
            if(m_conn_handle == BLE_CONN_HANDLE_INVALID) { // if not connected by APP
                LL_BLE_Adv_stop();
                update_device_name(LL_BLE_ADV_NAME);
                //LL_BLE_Adv_init(&init);
                LL_BLE_SetAdManu(LL_BLE_ADV_PAIRING_FLAG);
                #ifdef _LL_DELETE_BAS // if delete Battery Service
                    battery_level_update();
                #endif
                lumos_connectable = true; LL_BLE_Adv_start();
            }
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_FOR_APP_CONNECTING: {
            if(m_conn_handle == BLE_CONN_HANDLE_INVALID) { // if not connected by APP
                LL_BLE_Adv_stop();
                update_device_name(LL_BLE_ADV_NAME);
                //LL_BLE_Adv_init(&init);
                LL_BLE_SetAdManu(0x0000);
                #ifdef _LL_DELETE_BAS // if delete Battery Service
                    battery_level_update();
                #endif
                lumos_connectable = true; LL_BLE_Adv_start(); //timeslot_sd_start();
            }
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        //
        // new ones for Lumos devices
        case FUNC__START_ADV_MSG_Pairing: {
            adv__Pairing();
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_Lumos_Lights_State: {
            unsigned long lights_state; switch(gtSysState.eTurnState) {
                case TURNING_L:     lights_state = LIGHT_ON__turning_L |                                        LIGHT_ON__front_R | LIGHT_ON__rear_R ; break;
                case TURNING_R:     lights_state = LIGHT_ON__turning_R | LIGHT_ON__front_L | LIGHT_ON__rear_L                                        ; break;
                case TURNING_NONE:  lights_state =                       LIGHT_ON__front_L | LIGHT_ON__rear_L | LIGHT_ON__front_R | LIGHT_ON__rear_R ; break;
                default:            lights_state = 0; break; // should not happen
            }                
            adv__Lumos_Lights_State(lights_state);
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_Battery: {
            unsigned long battery_level = LL_Battery_Level();
            adv__Battery(battery_level);
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_Sync_Clock: {
            adv__Sync_Clock();
            LL_CLOCK__clearTheSnapshotAsMaster(); // now I'm the sync master, so no need to consider the "diff"
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_Power_ONOFF_ON: {
            adv__Power_ONOFF(POWER_STATE__ON);
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_Power_ONOFF_OFF: {
            adv__Power_ONOFF(POWER_STATE__OFF);
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_TurningAndBraking: {
            adv__TurningAndBraking(gtSysState.eTurnState, gtSysState.eBrakeState);
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        case FUNC__START_ADV_MSG_FlashingPattern: {
            unsigned char sender = (unsigned char)E_SYNC_MSG__FROM_HELMET;
            unsigned long mode = gtSysState.eModeOfWarningLight;
            adv__FlashingPattern(sender, LL_Para__get_flashing_pattern(mode, E_LL_LED_FRONT), LL_Para__get_flashing_pattern(mode, E_LL_LED_REAR));
        }; step++; time = gulTimerCnt1ms; break; // go to next step
        //
        // switch thread
        case FUNC__SWITCH_TO_THREAD__Pairing_APP: {
            LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Pairing_APP);
        }; break; // DO NOT MODIFY "steps" has been updated to another new steps
        case FUNC__SWITCH_TO_THREAD__Normal_Mode: {
            LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode);
        }; break; // DO NOT MODIFY "steps" has been updated to another new steps
        case FUNC__SWITCH_TO_THREAD__Normal_Mode_Sync_Clock: {
            if(LL_Timer_Elapsed_ms(last_time_of_syncing_or_being_synced) > (1*60*1000)) { // if timeout (then can sync now)
                LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode_Sync_Clock); last_time_of_syncing_or_being_synced = gulTimerCnt1ms;
            } else { // else (no need sync too frequently, so skip the sync and go to Normal Mode directly)
                LL_BLE_Broadcaster__start(thread__LL_BLE_Broadcaster__Normal_Mode);                
            }
        }; break; // DO NOT MODIFY "steps" has been updated to another new steps
        //
        default: break;
    }
}

