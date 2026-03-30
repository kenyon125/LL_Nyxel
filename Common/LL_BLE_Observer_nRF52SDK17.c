/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include "nordic_common.h"
//#include "nrf_sdh.h"
//#include "nrf_sdh_soc.h"
//#include "nrf_sdh_ble.h"
//#include "peer_manager.h"
//#include "peer_manager_handler.h"
//#include "app_timer.h"
//#include "bsp_btn_ble.h"
//#include "ble.h"
//#include "ble_advdata.h"
//#include "ble_advertising.h"
//#include "ble_conn_params.h"
//#include "ble_gap.h"
////#include "ble_db_discovery.h"
////#include "ble_hrs.h"
////#include "ble_rscs.h"
////#include "ble_hrs_c.h"
////#include "ble_rscs_c.h"
////#include "ble_conn_state.h"
////#include "nrf_fstorage.h"
////#include "fds.h"
////#include "nrf_ble_gatt.h"
////#include "nrf_ble_qwr.h"
////#include "nrf_pwr_mgmt.h"
//#include "nrf_ble_scan.h"

////#include "nrf_log.h"
////#include "nrf_log_ctrl.h"
////#include "nrf_log_default_backends.h"

//#include "LL_Clock.h" // get clock for sync
//#include "LL_Flash.h"
//#include "LL_Para.h"
//#include "LL_MSG_Handle_Helmet.h"
//#include "LL_BLE.h"
//#include "LL_Sync.h"
//#include "LL_BLE_Broadcaster.h"
#include "LL_Platform.h"
#include "LL_Common.h"
extern unsigned long sgulGeneralStep;

bool wake_up_event__power_on_cmd_scanned = false;
bool wake_up_event__key_pressed          = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NRF_BLE_SCAN_DEF(m_scan); /**< Scanning module instance. */

static ble_gap_scan_params_t m_normal_mode_scan_param = { /**< Scan parameters requested for scanning and connection. */
    .active        = 0x01,
    .interval      = 0x00D0,    // 0x00A0/0x00B0/0x00D0: 100ms/110ms/130ms, 130ms scanning can "meet" the 100ms adv earlier than 100ms scanning
    .window        = 0x003C,
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
    .timeout       = 0, // Duration of a scanning session in units of 10 ms. Range: 0x0001 - 0xFFFF (10 ms to 10.9225 ms). If set to 0x0000, the scanning continues until it is explicitly disabled. 
    .scan_phys     = BLE_GAP_PHY_1MBPS,
    .extended      = true,
};

static ble_gap_scan_params_t m_low_power_mode_scan_param = { /**< Scan parameters requested for scanning and connection. */
    .active        = 0x01,
		.interval    	 = 0x06E0,  // 6E0 means 1100ms // Determines scan interval in units of 0.625 millisecond.
		.window      	 = 0x0080,  // 80 means 80ms,
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
    .timeout       = 0, // Duration of a scanning session in units of 10 ms. Range: 0x0001 - 0xFFFF (10 ms to 10.9225 ms). If set to 0x0000, the scanning continues until it is explicitly disabled. 
    .scan_phys     = BLE_GAP_PHY_1MBPS,
    .extended      = true,
};

static void scan_evt_handler(scan_evt_t const * p_scan_evt) { // will called by "nrf_ble_scan_on_adv_report" called by "nrf_ble_scan_on_ble_evt" defined in "NRF_SDH_BLE_OBSERVER'
//  ret_code_t err_code;
    
    ble_gap_evt_adv_report_t const * p_adv        = p_scan_evt->params.filter_match.p_adv_report;
//  ble_gap_scan_params_t    const * p_scan_param = p_scan_evt->p_scan_params;
    switch(p_scan_evt->scan_evt_id) {
        case NRF_BLE_SCAN_EVT_FILTER_MATCH: { // if((p_adv->rssi < -80) || (p_adv->rssi > 80)) break;
//            const ble_gap_evt_t * p_gap_evt = &p_scan_evt->evt.gap_evt; {
//                const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report; {
                    uint8_t *p_adv_data = (uint8_t *)p_adv->data.p_data; {
                        uint32_t index = 0; while (index < p_adv->data.len) {
                            uint8_t field_length   =   p_adv_data[index   ];
                            uint8_t field_type     =   p_adv_data[index+1 ];
                            uint8_t *p_field_data  = &(p_adv_data[index+2 ]);
//                          uint8_t field_data_len = field_length - 1; // 1: 1 byte for "field_type"
                            {
															  if(gulCharging == 1)break;
                                if(field_type == BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME) {
                                    if('L' != p_field_data[0]) break;
                                    if('M' != p_field_data[1]) break;
                                    if('M' != p_field_data[2]) break;
                                }
                                if(field_type == BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA) {
                                    //
                                    // check Company ID
                                    if( (LL_BLE_COMPANY_ID_OF_LUMOS__BYTE_L != p_field_data[0]) 
                                    ||  (LL_BLE_COMPANY_ID_OF_LUMOS__BYTE_H != p_field_data[1]) ) { // (likely) if not Lumos 
                                        break; // ignore this Manufacturer Data
                                    } else {} // else, it's Lumos, continue
                                    //
                                    // get Group ID and Device Type
                                    unsigned char advPacketID[4]; memcpy(advPacketID, &(p_field_data[4]), 4);
                                    unsigned char device_type; device_type = p_field_data[9];
                                    bool is_paired_device = (memcmp(advPacketID, gtPara.netID, sizeof(advPacketID)) == 0);                                    //
                                    // parse
                                    if(0x1 == p_field_data[2]) { // 0x1: Pairing
                                        //LL_Helmet_ChangeStateTo_Pairing();
                                        if(SYS_PAIRING == gtSysState.eOnOFF){ // pairing mode
                                            memcpy(&(gtPara.netID), &(p_field_data[4]), LL_ESB_BASE_ADDR_LEN); gulFlashStoreNeeded = 1;
                                            LL_Helmet_ChangeStateTo_ON();
                                        }                                            
                                    } else if(0x8 == p_field_data[2]) { // 0x8: Flashing Pattern 
                                        //
                                        if(gtPara.publicNetworkEnabled) { // if Team Sync (then no need check ID)
                                            // nothing here
                                        } else { // else, still need check ID
                                            if(!is_paired_device) break; // not from a paired device
                                        }
                                        //
                                        { // copy from LL_MSG_Handle__PublicMessage_parseWhenInMainloop
                                            unsigned long sender               = p_field_data[10];//eBroadcastMsgPublic.message_type;
                                            unsigned long patternOfFrontOrDual = (((unsigned long)p_field_data[11]) <<  0)
                                                                               + (((unsigned long)p_field_data[12]) <<  8)
                                                                               + (((unsigned long)p_field_data[13]) << 16);
                                            unsigned long patternOfBack        = (((unsigned long)p_field_data[14]) <<  0)
                                                                               + (((unsigned long)p_field_data[15]) <<  8)
                                                                               + (((unsigned long)p_field_data[16]) << 16);
                                            //
                                            if(E_SYNC_MSG__FROM_HELMET == sender) {
                                                LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);
                                                LL_Para__set_flashing_pattern_fromSync(patternOfBack,        gtPara.eModeOfWarningLight, E_LL_LED_REAR);
                                                LL_Para__set_flashing_pattern_fromSync(patternOfBack,        gtPara.eModeOfWarningLight, E_LL_LED_REAR);                    
                                            }
                                            if(E_SYNC_MSG__FROM_FIREFLY_FRONT == sender) {
                                                LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);                    
                                                LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_REAR);    // "Follow"
                                                LL_Para__set_flashing_pattern_fromSync(patternOfFrontOrDual, gtPara.eModeOfWarningLight, E_LL_LED_REAR);    // "Follow"
                                            }
                                            if(E_SYNC_MSG__FROM_FIREFLY_REAR == sender) {
                                                LL_Para__set_flashing_pattern_fromSync(patternOfBack, gtPara.eModeOfWarningLight, E_LL_LED_FRONT);  // "Follow"
                                                LL_Para__set_flashing_pattern_fromSync(patternOfBack, gtPara.eModeOfWarningLight, E_LL_LED_REAR);
                                                LL_Para__set_flashing_pattern_fromSync(patternOfBack, gtPara.eModeOfWarningLight, E_LL_LED_REAR);                    
                                            }                                            
                                            //
                                            if(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight] <= LED_ANIMATION_FASTFLASH_MODE){ //common flash pattern
                                                LL_Drv_Ws2812_SetFrontAnimation(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight]);
                                                LL_Drv_Ws2812_SetRearAnimation(gtPara.ePatternOfRearLight[gtPara.eModeOfWarningLight]); 
                                            }                                            
                                        }
                                    } else { // else, other messages (which need check piaring ID)
                                        if(!is_paired_device) break; // not from a paired device
                                        if(0x6 == p_field_data[2]) { // 0x6: Power ON/OFF command
                                            if(0 == p_field_data[10]) { // 0: OFF
                                                if(SYS_ON != gtSysState.eOnOFF) break; // should not work if not Normal Mode
                                                {
                                                    LL_Helmet_ChangeStateTo_BeepBeforeOFF(); 
                                                }
                                            } else if(1 == p_field_data[10]) { // 1: ON
                                                if(SYS_OFF != gtSysState.eOnOFF) break; // should not work if not Low Power Mode
                                                extern unsigned long sgulGeneralStep;
                                                if(sgulGeneralStep != 6) break; // not work if not waiting for event
                                                {
                                                    wake_up_event__power_on_cmd_scanned = true;
                                                }      
                                            }                                                    
                                        } else if(0x7 == p_field_data[2]) { // 0x7: Turning command
                                            if( (0x40 != (device_type&0xF0))        // if not Lumos Remotes
                                            &&  (0xB0 != (device_type&0xF0)) ) {    // and not Controller devices
                                                break; // not work if command not from Controller
                                            }
//                                          if(SYS_PAIRING == gtSysState.eOnOFF) break; // no need work while pairing
                                            if(SYS_OFF     == gtSysState.eOnOFF) break; // no need work while Low Power                                            
                                            //
                                            //gtSysState.eFlashTypeOfBackLight = (E_FlashTypeOfBackLight)(p_field_data[10]);
                                            #if 1
                                            {
                                                #define TURNING_CMD_NONE      0
                                                #define TURNING_CMD_L         1
                                                #define TURNING_CMD_R         2
                                                #define BRAKING_CMD_NONE      0
                                                #define BRAKING_CMD_BRAKING   1                                                                                                
                                                unsigned char turning_cmd = (p_field_data[10] & 0xF0) >> 4;
                                                unsigned char braking_cmd = (p_field_data[10] & 0x0F) >> 0;
                                                if(TURNING_CMD_NONE == turning_cmd) { // if Turning done
                                                    gtSysState.eTurnState  = TURNING_NONE; 
                                                    gtSysState.eBrakeState = (braking_cmd == 1) ? BRAKE_ON : BRAKE_OFF;
                                                } else if(TURNING_CMD_L == turning_cmd) { // if Turning L
                                                    gtSysState.eTurnState = TURNING_L;                    
                                                    gtSysState.eBrakeState = BRAKE_OFF; // no need braking since turning is prior
                                                } else if(TURNING_CMD_R == turning_cmd) { // if Turning R
                                                    gtSysState.eTurnState = TURNING_R;                      
                                                    gtSysState.eBrakeState = BRAKE_OFF; // no need braking since turning is prior
                                                } else { // exception
                                                    // just ignore
                                                }
                                                //
                                                LL_HelmetActionWhenStateChanged();
                                            }
                                            #endif
                                        } else if(0x5 == p_field_data[2]) { // 0x5: Sync Clock
                                            if(SYS_ON != gtSysState.eOnOFF) break; // should not work if not Normal Mode
                                            {
                                                unsigned long clock_in_adv_from_others; {
                                                    unsigned long bit07to00  = p_field_data[10]; bit07to00 = bit07to00 <<  0;
                                                    unsigned long bit15to08  = p_field_data[11]; bit15to08 = bit15to08 <<  8;
                                                    unsigned long bit23to16  = p_field_data[12]; bit23to16 = bit23to16 << 16;
                                                    clock_in_adv_from_others = bit07to00
                                                                             | bit15to08
                                                                             | bit23to16;
                                                }
                                                LL_CLOCK__updateTheSnapshotAsSlave(clock_from_ble_evt_dispatch, clock_in_adv_from_others); last_time_of_syncing_or_being_synced = gulTimerCnt1ms;
                                            }
                                        }
                                    }
                                }
                            }
                        index += field_length + 1; }
                    }
//                }
//            }            
        } break;
        default: break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LL_BLE_Observer__Init(void) { // acc to scan_init and scan_start of SDK17 sample ble_app_hrs_rscs_relay
    ret_code_t err_code;
    
//  nrf_ble_scan_stop();
    
    { // scan_init
        nrf_ble_scan_init_t init_scan; {
            memset(&init_scan, 0, sizeof(init_scan));
            init_scan.p_scan_param = &m_normal_mode_scan_param;
        }
        err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler); APP_ERROR_CHECK(err_code);
        err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, "LMM"); APP_ERROR_CHECK(err_code);
//      err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &m_adv_uuids[HART_RATE_SERVICE_UUID_IDX]); APP_ERROR_CHECK(err_code);
        err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_ALL_FILTER, false); APP_ERROR_CHECK(err_code);        
    }
    
    { // scan_start
        err_code = nrf_ble_scan_start(&m_scan); APP_ERROR_CHECK(err_code);
    }
}

void LL_BLE_Observer__Stop(void)
{
	nrf_ble_scan_stop();
}

void LL_BLE_Observer(void) {
}

void LL_BLE_Observer__Modify_Param(E_SCAN_PARAM_TYPE param)
{
		uint32_t err_code;
		nrf_ble_scan_stop();
		if(param == TYPE_NORMAL){
			err_code = sd_ble_gap_scan_start(&m_normal_mode_scan_param, &m_scan.scan_buffer);
		}else{
			err_code = sd_ble_gap_scan_start(&m_low_power_mode_scan_param, &m_scan.scan_buffer);
		}			
		APP_ERROR_CHECK(err_code);	
}
