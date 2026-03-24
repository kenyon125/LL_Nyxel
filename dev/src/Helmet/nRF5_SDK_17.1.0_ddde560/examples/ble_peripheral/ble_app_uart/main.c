/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "ble_dis.h"
#include "ble_bas.h"
#include "ble_dfu.h"
#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"
#include "nrf_power.h"
#include "nrf_bootloader_info.h"

#include "LL_Key.h"
#include "LL_GPIO.h"
#include "LL_Battery.h"
#include "LL_BatteryIndicator.h"
#include "LL_Battery_Charge.h"
#include "LL_ADC.h"
#include "LL_Timer.h"
#include "nrf_delay.h"
#include "LL_LED_Panel_WS2812.h"
#include "LL_MSG.h"
#include "LL_PWM.h"
#include "LL_LED_Helmet.h"
#include "LL_SysMode.h"
#include "LL_Flash.h"
#include "esb_timeslot.h"
#include "LL_Para.h"
#include "LL_Sync.h"
#include "LL_Interface_withApp.h"
#include "LL_MSG_Handle_Helmet.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "LL_BLE.h"
#include "LL_BLE_Broadcaster.h"
#include "LL_BLE_Observer.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define MANUFACTURER_NAME               "LumenLabs"                                  /**< Manufacturer. Will be passed to Device Information Service. */

#define DEVICE_NAME                     "Nyxel"                                  /**< Name of device. Will be included in the advertising data. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                800                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                180                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(((LL_TIMESLOT_DISTANCE/1000)*1)+ 5, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(((LL_TIMESLOT_DISTANCE/1000)*1)+20, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
BLE_BAS_DEF(m_bas);		
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
#if 1 // to wrap the internal "m_advertising" so that other files can use the advertising init/start/stop
void LL_BLE_Adv_init(ble_advertising_init_t const * const p_init) {
    uint32_t err_code = ble_advertising_init(&m_advertising, p_init); APP_ERROR_CHECK(err_code);
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);            
}
void LL_BLE_Adv_start(void) {
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST); APP_ERROR_CHECK(err_code);
}
void LL_BLE_Adv_stop(void) {
    sd_ble_gap_adv_stop(m_advertising.adv_handle);
}
#endif
bool glBleConnected = false;
ble_advdata_t scanrsp;
uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE},
		//{BLE_UUID_BATTERY_SERVICE,BLE_UUID_TYPE_BLE},
		{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

APP_TIMER_DEF(m_1ms_timer_id);                                                      /**< 1ms timer. */

typedef struct
{
	char first_byte_high_4bit;
	char first_byte_low_4bit;
	char second_byte_high_4bit;
	char second_byte_low_4bit;
}T_MSG_Data_Analysis;
T_MSG_Data_Analysis gtMsgDateFormat;
unsigned long gulDisconnectRountine = 0;
unsigned long gulDisconnectCnt = 0;
unsigned long gulTimerCnt1ms;

unsigned long gulRemoteBatteryAutoUpdate = 1;
uint8_t AppViaPairingMode = 0; // 1:App via command enter pairing mode
unsigned char BleCommandNeedDealInLoop = 0;
signed char turning_state = 0;
signed char braking_state = 0;
unsigned char needExitPairingMode = 0;
T_LL_HardwareID gtHwID;
/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    //NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}


unsigned long batteryLevelTotal = 0;
uint8_t batCnt = 0;
/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
void battery_level_update(void)
{
    ret_code_t err_code;
	
    uint8_t battery_level = LL_Battery_Level();
      
    batteryLevelTotal += battery_level;
    batCnt++;
    
    if(batCnt == 15){
        if( (batteryLevelTotal/batCnt <= 3) ){
            LL_BatteryIndicator__Start();  
        }      
        batteryLevelTotal = 0;
        batCnt = 0;
    }
    
    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

static unsigned long sgulCalibrate = 0;
void clock1ms_timeout_handler(void * p_context) {
    gulTimerCnt1ms++; sgulCalibrate++; if( 142 == sgulCalibrate) { gulTimerCnt1ms++; sgulCalibrate = 0; }
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_create(&m_1ms_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                clock1ms_timeout_handler);
    APP_ERROR_CHECK(err_code);
		
	app_timer_start(m_1ms_timer_id,33,NULL);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);
 
    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}

static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));
 
    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static unsigned long sgulNusDataHandleNeeded = 0; // 0: no,  others: length.
#define received_ble_message    sgulNusDataHandleNeeded
#define ble_message_length      sgulNusDataHandleNeeded
#define ble_message             sgaucNusRcvBuf
#define NUS_RCV_BUF_LEN 32
static unsigned char sgaucNusRcvBuf[NUS_RCV_BUF_LEN];
uint8_t data_length = 0;
void TxFlashModeParaToApp__Start(void);
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    data_length = p_evt->params.rx_data.length;
    if(0 == sgulNusDataHandleNeeded)
		{
        if(NUS_RCV_BUF_LEN >= data_length) {
            memcpy(sgaucNusRcvBuf, p_evt->params.rx_data.p_data, data_length);
            sgulNusDataHandleNeeded = data_length;
        } else {
            // Error: buffer size not enough!
        }
        
    } else {
        // Error: more buffer or faster handle needed!
    }

}

//void TxFlashModeParaToApp__Start(void);
void IndicatorOfBrakeFuncOnOff__Start(void);
#define handle_ble_pairing_message_from_app     nus_data_handler_real()
#define handle_ble_normalMode_message_from_app  nus_data_handler_real()
static void nus_data_handler_real(void)
{
		if(0 == sgulNusDataHandleNeeded) return;
	
		uint8_t * p_data = sgaucNusRcvBuf;
		uint16_t length  = sgulNusDataHandleNeeded;
		uint16_t SendDataLen = 0;
	
    if(2 == length) {
        if( ('O' == p_data[0]) && ('N' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_NONE;
            gtSysState.eBrakeState = BRAKE_OFF;
            LL_HelmetActionWhenStateChanged();
        } else if( ('O' == p_data[0]) && ('K' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_NONE;
            gtSysState.eBrakeState = BRAKE_ON; // no need to check the ON/OFF of brake function
            LL_HelmetActionWhenStateChanged();
        } else if( ('L' == p_data[0]) && ('N' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_L;
            gtSysState.eBrakeState = BRAKE_OFF;
            LL_HelmetActionWhenStateChanged();
        } else if( ('L' == p_data[0]) && ('K' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_L;
            gtSysState.eBrakeState = BRAKE_ON; // no need to check the ON/OFF of brake function
            LL_HelmetActionWhenStateChanged();
        } else if( ('R' == p_data[0]) && ('N' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_R;
            gtSysState.eBrakeState = BRAKE_OFF;
            LL_HelmetActionWhenStateChanged();
        } else if( ('R' == p_data[0]) && ('K' == p_data[1]) ) {
            gtSysState.eTurnState = TURNING_R;
            gtSysState.eBrakeState = BRAKE_ON; // no need to check the ON/OFF of brake function
            LL_HelmetActionWhenStateChanged();
        } else {
            // other cmd
        }
    }
//sgulNusDataHandleNeeded = 0;			
		char string[24]; 
    // ON/OFF brake function
    if(2 == length) {
        if(      ('B' == p_data[0]) && ('O' == p_data[1]) ) { gtPara.ulBrakeFunction = 1; gulFlashStoreNeeded = 1; IndicatorOfBrakeFuncOnOff__Start(); }
        else if( ('B' == p_data[0]) && ('C' == p_data[1]) ) { gtPara.ulBrakeFunction = 0; gulFlashStoreNeeded = 1; IndicatorOfBrakeFuncOnOff__Start(); }
        else if( ('f' == p_data[0]) && ('?' == p_data[1]) ) { 
            sprintf(string, "f%u", gtPara.eModeOfWarningLight); 
            SendDataLen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);          }
        else if( ('B' == p_data[0]) && ('S' == p_data[1]) ) { 
            if(0 == gtPara.ulBrakeFunction) { sprintf(string, "BC:%+06d", gtPara.ssThreshhold); }
            else                            { sprintf(string, "BO:%+06d", gtPara.ssThreshhold); }
            SendDataLen = 9;
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);                  }
        else if( ('B' == p_data[0]) && ('L' == p_data[1]) ) { 
            unsigned long battery_level = LL_Battery_Level();
            //char string[8]; 
            if(LL_BATTERY_LEVEL_NONE != battery_level) { sprintf(string, "BL:%03u\0", battery_level); }
            else                                       { sprintf(string, "BL:N/A\0");               }
            SendDataLen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);  
        } 
        else if( ('S' == p_data[0]) && ('N' == p_data[1]) ) { gtPara.eBeepMode = BEEP_NONE;
            T_BUZZER_BEEP beep_type = BUZZER_BEEP__NONE;
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]), &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]), &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]),   &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]),   &beep_type, sizeof(T_BUZZER_BEEP));
            //
            gulFlashStoreNeeded = 1; } 
        else if( ('S' == p_data[0]) && ('O' == p_data[1]) ) { gtPara.eBeepMode = BEEP_WHEN_TURNING_ONOFF;    
            T_BUZZER_BEEP beep_type = BUZZER_BEEP__ONCE;
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]), &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]), &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]),   &beep_type, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]),   &beep_type, sizeof(T_BUZZER_BEEP));
            //
            gulFlashStoreNeeded = 1; } 
        else if( ('S' == p_data[0]) && ('C' == p_data[1]) ) { gtPara.eBeepMode = BEEP_EVERY_1_TURNING_FLASH; 
            T_BUZZER_BEEP beep_type_begin = BUZZER_BEEP__PER_0p5S;
            T_BUZZER_BEEP beep_type_end   = BUZZER_BEEP__NONE;
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            //
            gulFlashStoreNeeded = 1; } 
        else if( ('S' == p_data[0]) && ('H' == p_data[1]) ) { gtPara.eBeepMode = BEEP_EVERY_4_TURNING_FLASH; 
            T_BUZZER_BEEP beep_type_begin = BUZZER_BEEP__PER_2S;
            T_BUZZER_BEEP beep_type_end   = BUZZER_BEEP__NONE;
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            //
            gulFlashStoreNeeded = 1; } 
        else if( ('S' == p_data[0]) && ('L' == p_data[1]) ) { gtPara.eBeepMode = BEEP_EVERY_8_TURNING_FLASH; 
            T_BUZZER_BEEP beep_type_begin = BUZZER_BEEP__PER_2S;
            T_BUZZER_BEEP beep_type_end   = BUZZER_BEEP__NONE;
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_BEGIN]), &beep_type_begin, sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__L_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            memcpy(&(gtPara.beep_of_turning[E_BEEP_OF_TURNING__R_END]),   &beep_type_end,   sizeof(T_BUZZER_BEEP));
            //
            gulFlashStoreNeeded = 1; } 
        else if(0 == memcmp("B0", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__MUTE; gtPara.eBeepMode = BEEP_NONE;                  gulFlashStoreNeeded = 1;} //LL_BeepForTurning__ON(); } 
        else if(0 == memcmp("B1", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__BEEP; gtPara.eBeepMode = BEEP_NONE;                  gulFlashStoreNeeded = 1;} //LL_BeepForTurning__ON(); } 
        else if(0 == memcmp("B2", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__BEEP; gtPara.eBeepMode = BEEP_WHEN_TURNING_ONOFF;    gulFlashStoreNeeded = 1;} //LL_BeepForTurning__ON(); } 
        else if(0 == memcmp("B3", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__BEEP; gtPara.eBeepMode = BEEP_EVERY_1_TURNING_FLASH; gulFlashStoreNeeded = 1;} //LL_BeepForTurning__ON(); } 
				else if(0 == memcmp("P0", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__MUTE; gulFlashStoreNeeded = 1; } 
        else if(0 == memcmp("P1", p_data, 2)) { gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__BEEP; gulFlashStoreNeeded = 1; } 
    } else if(3 == length) {
        if( ('B' == p_data[0]) && ('L' == p_data[1]) && ('O' == p_data[2]) ) { gulRemoteBatteryAutoUpdate = 1; }
        else if( ('B' == p_data[0]) && ('L' == p_data[1]) && ('C' == p_data[2]) ) { gulRemoteBatteryAutoUpdate = 0; }
        else if('Z' == p_data[0]) { // buzzer setting, "Zmn", while "m" is 0/1/2/3 menas L(begin)/R(begin)/L(end)/R(end), n = 0~5, means None/Once/0.5s/2s/Car-Blinker/Woodcock
            //
            LL_LED_OFF(E_LL_PWM_BUZZER); // turn off the "Turning Off" beeping
            //
            {
                unsigned long m = p_data[1] - '0';
                unsigned long n = p_data[2] - '0';
                T_BUZZER_BEEP beep_types[] = {
                    BUZZER_BEEP__NONE, 
                    BUZZER_BEEP__ONCE, 
                    BUZZER_BEEP__PER_0p5S, 
                    BUZZER_BEEP__PER_2S, 
                    BUZZER_BEEP__1HZ, 
                    BUZZER_BEEP__WOODCOCK,
                    BUZZER_BEEP__TWICE_PER_2S
                };
                memcpy(&(gtPara.beep_of_turning[m]), &(beep_types[n]), sizeof(T_BUZZER_BEEP)); gulFlashStoreNeeded = 1;
            }
            //
            if(gtSysState.eTurnState != TURNING_NONE) { // if turning
                gtSysState_prev.eTurnState = TURNING_NONE; // trigger the "LL_HelmetActionWhenStateChanged" actually work
                LL_HelmetActionWhenStateChanged();            
            }            
        }   
		} else if(4 == length) {			
        if( ('B' == p_data[0]) && 
            ('L' == p_data[1]) &&
            ('-' == p_data[2]) &&
            ('R' == p_data[3])    ) {
            //char string[16]; 
            if(0xCCCCCCCC == gulBatteryOfRemote) { // if Remote Lite
                sprintf(string, "BL-R:Lite\0");
            } else { // else, Lumos Remote
                if(0xFFFFFFFF != gulBatteryOfRemote) { sprintf(string, "BL-R:%03u\0", gulBatteryOfRemote); }
                else                                 { sprintf(string, "BL-R:NA\0");                    }
            }
			SendDataLen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);  
        } else if( ('V' == p_data[0]) && ('E' == p_data[1]) && ('R' == p_data[2]) && ('H' == p_data[3]) ) { 
            sprintf(string, "%04d-%02d-%02d"        ,
                ((LL_VER_DATE&0xF000)>>12) + 2016,
                ((LL_VER_DATE&0x0F00)>> 8)       ,
                ((LL_VER_DATE&0x00FF)    )       );
			SendDataLen = 10;
			ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
        } else if( ('V' == p_data[0]) && ('E' == p_data[1]) && ('R' == p_data[2]) && ('R' == p_data[3]) ) { 
            if(sgtMsg.cStateBrake == 'C') {
                    sprintf(string, "RL-V0.5");							
            }else{				
                if(sgtMsg.usVerDate != 0x0000){
                    uint8_t year,fw_version_h,fw_version_l,model_h;
                    gtMsgDateFormat.first_byte_high_4bit = ((sgtMsg.usVerDate)&0xF000)>>12;	
                    gtMsgDateFormat.first_byte_low_4bit = ((sgtMsg.usVerDate)&0x0F00)>>8;	
                    gtMsgDateFormat.second_byte_high_4bit = ((sgtMsg.usVerDate)&0x00F0)>>4;	
                    gtMsgDateFormat.second_byte_low_4bit = (sgtMsg.usVerDate)&0x000F;
                    
                    if(gtMsgDateFormat.second_byte_high_4bit > 0x01){ // >0x01 indicates not the date but the model
                        fw_version_h = gtMsgDateFormat.first_byte_high_4bit; 
                        fw_version_l = gtMsgDateFormat.first_byte_low_4bit;
                        model_h = gtMsgDateFormat.second_byte_high_4bit;
                        //model_l = gtMsgDateFormat.second_byte_low_4bit;
                        if(model_h == 0x0A)//model_h = LL_MODEL_H*10 in order to distinguish from the date
                        { //remote1.0
                            sprintf(string, "RN51-V%d.%d",fw_version_h, fw_version_l);
                        }else if(model_h == 0x02){ //remote2.0
                            sprintf(string, "RN52-V%d.%d",fw_version_h, fw_version_l);
                        }else if(model_h == 0x04){ //remote4.0 remote_USBC										
                            sprintf(string, "RU-V%d.%d",fw_version_h, fw_version_l);
                        }
                    }else{
                        year = gtMsgDateFormat.first_byte_high_4bit; //year:0~F
                        //month = gtMsgDateFormat.first_byte_low_4bit;	//month:1-C
                        if(year <= 0x01){	//2016-2017
                            sprintf(string,"RN51-V2.5");
                        }else{
                            sprintf(string,"RN52-V1.3");
                        }
                    }
                }else{
                        sprintf(string,"UNKRMT");
                }
            }
            SendDataLen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
        } else if( 
            ('T' == p_data[0]) && 
            ('R' == p_data[1]) &&
            ('S' == p_data[2]) &&
            ('N' == p_data[3])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FRSN");
                    memcpy(&string[4], &(gtPara.aucSN[0]), 14);
                    SendDataLen = 18;
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);  
                    //LL_Helmet_CmdTestingMode_CMD(LL_CMD_TESTING_RSNH);
                }
#endif                 
        } else if( ('B' == p_data[0]) && ('M' == p_data[1]) ) { 
            unsigned char mode       = p_data[2]; if(5 < mode) return;
            unsigned char brightness = p_data[3]; //if(100 < brightness) return;
            
            LL_Get_Color_Value_Via_Brightness(brightness, &gtPara.brightness_individual[mode][E_LL_LED_FRONT], &gtPara.brightness_individual[mode][E_LL_LED_REAR]);
            gulFlashStoreNeeded = 1; // !!!!!!
            LL_HelmetActionWhenStateChanged();
        } else if(0 == memcmp("SLP", p_data, 3)) {
            unsigned long option = p_data[3] - '0'; if(10 < option) { option = 10; } // 10: 10 10s which is 100s
            if(1 == canThisRemoteChangeSleepTime) {
                #if 0 // before 2.4GHz updating
                    gtMsgToRemote.ssThreshhold = (option<<8) + option; // make 2 bytes have the same value so that the receiver no need to check endian
                    { // seems not work, still need Remote press twice to take effect
                        extern unsigned long gulESB_NeedAckPayload;
                        gulESB_NeedAckPayload = 1;
                    }
                #else // new 2.4GHz module (interface)
                    option_SleepTime_of_remote_from_app = (option<<8) + option; // make 2 bytes have the same value so that the receiver no need to check endian
                #endif
                sprintf(string, "SLP changed.\0");
            } else {
                sprintf(string, "SLP NA!\0");
            } 
            SendDataLen = strlen(string);
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
        }else if(0 == memcmp("PUB", p_data, 3)) { char command = p_data[3]; // turn ON/OFF public network
            if('1' == command) { // if command "ON"
                gtPara.publicNetworkEnabled = 1; // 1: enabled
                gulFlashStoreNeeded = 1;
            } else if('0' == command) { // else if command "OFF"
                gtPara.publicNetworkEnabled = 0; // 0: disabled
                gulFlashStoreNeeded = 1;
            } else if('?' == command) { // else if command "read"
                if(1 == gtPara.publicNetworkEnabled) {
                    sprintf(string, "PUB1\0");
                } else {
                    sprintf(string, "PUB0\0");
                }
                SendDataLen = strlen(string);
                ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
            }
        }
    }else if(5 == length) {
        if('F' == p_data[0]) { // customer flashing mode cmd
                    // parse the cmd
                    unsigned long mode            ;//= p_data[1] >> 6; if(2 < mode) { mode = 0; }
                    unsigned long part            ;//= p_data[1] & 0x3F;
                    unsigned long timeslot_states ;//= (p_data[4]<<16) + (p_data[3]<<8) + p_data[2];
                    unpackFlashModeParaFromApp(p_data, &mode, &part, &timeslot_states);
                    if(LL_PART_BACK_LIGHT == part) {
                        LL_Para__set_flashing_pattern_fromApp(timeslot_states, mode, E_LL_LED_REAR);//gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_BACK_L] = timeslot_states;
                        //gulFlashStoreNeeded = 1;
                    } else if(LL_PART_FRONT_LIGHT == part) {
                        LL_Para__set_flashing_pattern_fromApp(timeslot_states, mode, E_LL_LED_FRONT);//gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_FRONT] = timeslot_states;
                        //gulFlashStoreNeeded = 1;
                    }

                    // switch to this mode immediately
                    switch(mode) {
                        case 0:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break;
                        case 1:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode1; break;
                        case 2:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode2; break;
                        default: gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break; // shd not happen
                    }
                    LL_Helmet_ChangeStateTo_NextMode();
        } else if('f' == p_data[0]) {
                    unsigned long mode ;//= p_data[1] - '0';
                    unpackCurrentModeFromApp(p_data, &mode);
                    // switch to this mode immediately
                    switch(mode) {
                        case 0:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break;
                        case 1:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode1; break;
                        case 2:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode2; break;
                        default: gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break; // shd not happen
                    }
                    LL_Helmet_ChangeStateTo_NextMode();
        } else if( 0 == memcmp("NO_PR", p_data, 5) ) { // not set peer data when DFU, for Nordic's Android 10 Note 9 DFU issue
            //extern unsigned long g_not_set_peer_data_when_DFU;kzc
           /// g_not_set_peer_data_when_DFU = 1;
        } else if(
            ('T' == p_data[0]) && 
            ('P' == p_data[1]) &&
            ('A' == p_data[2]) &&
            ('I' == p_data[3]) &&
            ('R' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if( (SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) || (4 == gulCmdTestingMode) ) {
                    sprintf(string, "FPAIR\0");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    // turn off the LED, then enter pairing mode:
                    LL_LEDs_OFF(); //LL_Helmet_RedGreenFlashWhenTestMode_OFF();
                    gulCmdTestingMode = 1; LL_Helmet_ChangeStateTo_Pairing();
                }
#endif                
        } else if( // TESTT
            ('T' == p_data[0]) && 
            ('E' == p_data[1]) &&
            ('S' == p_data[2]) &&
            ('T' == p_data[3]) &&
            ('T' == p_data[4])    ) {
                if(SYS_PAIRING == gtSysState.eOnOFF) {
                    gulDisconnectRountine = 0; // !!!!!!
                    // Entering test-mode
                    sprintf(string, "FESTT\0");
                    SendDataLen = 5;
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    LL_Helmet_ChangeStateTo_ProductionTestingMode();
                }
        } else if( 
            ('T' == p_data[0]) && 
            ('R' == p_data[1]) &&
            ('S' == p_data[2]) &&
            ('N' == p_data[3]) &&
            ('H' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FSNH");
                    memcpy(&string[4], &(gtPara.aucSN[0]), 10);
                    string[14] = '\0';
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    //LL_Helmet_CmdTestingMode_CMD(LL_CMD_TESTING_RSNH);
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('R' == p_data[1]) &&
            ('S' == p_data[2]) &&
            ('N' == p_data[3]) &&
            ('L' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FSNL");
                    memcpy(&string[4], &(gtPara.aucSN[10]), 10);
                    string[14] = '\0';
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    //LL_Helmet_CmdTestingMode_CMD(LL_CMD_TESTING_RSNH);
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('S' == p_data[1]) &&
            ('H' == p_data[2]) &&
            ('U' == p_data[3]) &&
            ('T' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if( (SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) || (4 == gulCmdTestingMode) ) {
                    sprintf(string, "FSHUT");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    nrf_delay_ms(100); LL_Helmet_ChangeStateTo_OFF();
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('B' == p_data[1]) &&
            ('R' == p_data[2]) &&
            ('E' == p_data[3]) &&
            ('1' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FBRE1");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    gtPara.ulBrakeFunction = 1; gulFlashStoreNeeded = 1;
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('B' == p_data[1]) &&
            ('R' == p_data[2]) &&
            ('E' == p_data[3]) &&
            ('0'== p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FBRE0");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    gtPara.ulBrakeFunction = 0; gulFlashStoreNeeded = 1;
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('R' == p_data[1]) &&
            ('X' == p_data[2]) &&
            ('Y' == p_data[3]) &&
            ('Z' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FRXYZ%+04d%+04d%+04d", gtPara.ssKxyz[0], gtPara.ssKxyz[1], gtPara.ssKxyz[2]);
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('G' == p_data[1]) &&
            ('R' == p_data[2]) &&
            ('T' == p_data[3]) &&
            ('H' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FGRTH%+06d", gtPara.ssThreshhold);
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                }
#endif                
        } else if( 
            ('T' == p_data[0]) && 
            ('R' == p_data[1]) &&
            ('E' == p_data[2]) &&
            ('S' == p_data[3]) &&
            ('T' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {                
                    sprintf(string, "FREST\0");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    gtPara.ulBrakeFunction = 0;
                    gtPara.ssKxyz[0] = 100;
                    gtPara.ssKxyz[1] =   0;
                    gtPara.ssKxyz[2] =   0;
                    gulFlashStoreNeeded = 1;
                }
#endif       
        } else if (0 == memcmp("sync0", p_data, 5)) { // if message is "sync0"
                if(gtPara.ePatternOfFrontLight[gtPara.eModeOfWarningLight] <= LED_ANIMATION_FASTFLASH_MODE){ //common flash pattern
                    geBroadcastState = E_BROADCAST_STATE__NEED_PTX; // sync
                }else{
                    geBroadcastState = E_BROADCAST_STATE__NEED_PTX_OnlyClock; // sync clock
                }                                          
        } else if( ('B' == p_data[0]) && ('I' == p_data[1]) ) { // Brightness Individual
            unsigned char mode       = p_data[2]; if(5 < mode) return;
            unsigned char light      = p_data[3]; if(E_LL_PWM_CH_NUM <= light || light == E_LL_PWM_BUZZER) return;
            unsigned char brightness = p_data[4]; //if(100 < brightness) return;
            
            LL_Get_Color_Value_Via_Individual_Brightness(brightness, &gtPara.brightness_individual[mode][light]);
            gulFlashStoreNeeded = 1; // !!!!!!
            LL_HelmetActionWhenStateChanged();
        }
    }else if(6 == length) {
        if(SYS_PAIRING == gtSysState.eOnOFF) { 
            if(0 == strncmp((const char *)(gtPara.scBleAddr), (const char *)(p_data), BLE_DEVICE_ADDR_LEN) ) {
                    // app ID match, not exit pairing mode.
                    TxFlashModeParaToApp__Start();
            } else {
                    memcpy(gtPara.scBleAddr, p_data, BLE_DEVICE_ADDR_LEN); 
                    //SEGGER_RTT_WriteString(0, "pairing: receive ID\n");
                    // exit pairing mode at once:
                    gulFlashStoreNeeded = 1;                            
                    LL_Helmet_ChangeStateTo_ON();
                    TxFlashModeParaToApp__Start();
            }
        } else {
            if(0 == strncmp((const char *)(gtPara.scBleAddr), (const char *)(p_data), BLE_DEVICE_ADDR_LEN) ) {
                gulDisconnectRountine = 0;
                //SEGGER_RTT_WriteString(0, "normal: ID match\n");
                TxFlashModeParaToApp__Start();
            } else {
                //SEGGER_RTT_WriteString(0, "normal: ID not match\n");
            }
        }
    }else if(8 == length) {
        if('X' == p_data[0]) { gtPara.ssKxyz[0]    = atoi((const char *)(&(p_data[1]))); gulFlashStoreNeeded = 1; }
        if('Y' == p_data[0]) { gtPara.ssKxyz[1]    = atoi((const char *)(&(p_data[1]))); gulFlashStoreNeeded = 1; }
        if('Z' == p_data[0]) { gtPara.ssKxyz[2]    = atoi((const char *)(&(p_data[1]))); gulFlashStoreNeeded = 1; }
        if('T' == p_data[0]) { gtPara.ssThreshhold = atoi((const char *)(&(p_data[1]))); gulFlashStoreNeeded = 1; }
    } else if(9 == length) {
        if(      0 == memcmp("PAIRING-Y", p_data, 9) ) { if(SYS_ON      == gtSysState.eOnOFF) { LL_Helmet_ChangeStateTo_Pairing(); } }
        else if( 0 == memcmp("PAIRING-N", p_data, 9) ) { if(SYS_PAIRING == gtSysState.eOnOFF) { LL_Helmet_ChangeStateTo_ON();      } }
				else if(0 == memcmp("POWEROFF", p_data, 8)) { char command = p_data[8]; // turn ON/OFF sync POWER off
						if('1' == command) { // if command "ON"
								setPara(syncPowerOff, 1); // 1: enabled
								gulFlashStoreNeeded = 1;
						} else if('0' == command) { // else if command "OFF"
								setPara(syncPowerOff, 0); // 0: disabled
								gulFlashStoreNeeded = 1;
						} else if('?' == command) { // else if command "read"
								if(1 == getPara(syncPowerOff)) {
										sprintf(string, "POWEROFF1\0");
								} else {
										sprintf(string, "POWEROFF0\0");
								}
								SendDataLen = strlen(string);
								ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
						}
					}
		} 
		else if((10) == length) { 
        if( 0 == memcmp("NETID ", p_data, 6) ) { // change the "paired app ID"
            memcpy(gtPara.netID, &(p_data[6]), LL_ESB_BASE_ADDR_LEN); gtPara.netID_exist = 1; gulFlashStoreNeeded = 1;
			geBroadcastState = E_BROADCAST_STATE__NEED_PTX_OnlyClock;//E_BROADCAST_STATE__NEED_PTX; // sync
            sprintf(string, "NETID changed\0");
						SendDataLen = strlen(string);
						ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
        }else if('A' == p_data[0] && 'N' == p_data[1] && 'C' == p_data[2]) { // customer flashing mode cmd
            // parse the cmd
            T_Animation_Data AnimationData;
            AnimationData.eMode = p_data[3];
            AnimationData.part = p_data[4];
            
            AnimationData.color_code = p_data[5];
            AnimationData.type = (E_Ws2812_Animation_Type)p_data[6];
            if(AnimationData.type == LL_TYPE_COMMON_ANIMATION){  //need flash_pattern
                AnimationData.flash_pattern  = (p_data[7]<< 16) + (p_data[8]<< 8) + (p_data[9]<< 0);
                if(LL_PART_BACK_LIGHT == AnimationData.part) {
                    LL_Para__set_flashing_pattern_fromApp(AnimationData.flash_pattern, AnimationData.eMode, E_LL_LED_REAR);
                    LL_Get_Color_Value_Via_Color(AnimationData.color_code, &gtPara.brightness_individual[AnimationData.eMode][E_LL_LED_REAR]);
                    gtPara.ePatternOfRearLight[AnimationData.eMode] = AnimationData.type;
                    //gtPara.brightness_individual[AnimationData.eMode][E_LL_LED_REAR] = AnimationData.color_code;
                    gulFlashStoreNeeded = 1;
                } else if(LL_PART_FRONT_LIGHT == AnimationData.part) {
                    LL_Para__set_flashing_pattern_fromApp(AnimationData.flash_pattern, AnimationData.eMode, E_LL_LED_FRONT);
                    LL_Get_Color_Value_Via_Color(AnimationData.color_code, &gtPara.brightness_individual[AnimationData.eMode][E_LL_LED_FRONT]);
                    gtPara.ePatternOfFrontLight[AnimationData.eMode] = AnimationData.type;
                    //gtPara.brightness_individual[AnimationData.eMode][E_LL_LED_FRONT] = AnimationData.color_code;
                    gulFlashStoreNeeded = 1;
                }                
            }else{
                #define Animation_FlashPattern 0x00FFFFFF
                AnimationData.type += 2;//ignore solid+slow+flash 
                if(AnimationData.type < LED_ANIMATION_COMMON_MODE_NUM){
                    if(LL_PART_BACK_LIGHT == AnimationData.part) {
                        LL_Para__set_flashing_pattern_fromApp(Animation_FlashPattern, AnimationData.eMode, E_LL_LED_REAR);//gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_BACK_L] = timeslot_states;
                        gtPara.ePatternOfRearLight[AnimationData.eMode] = AnimationData.type;
                        gulFlashStoreNeeded = 1;
                    } else if(LL_PART_FRONT_LIGHT == AnimationData.part) {
                        LL_Para__set_flashing_pattern_fromApp(Animation_FlashPattern, AnimationData.eMode, E_LL_LED_FRONT);//gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_FRONT] = timeslot_states;
                        gtPara.ePatternOfFrontLight[AnimationData.eMode] = AnimationData.type;
                        gulFlashStoreNeeded = 1;
                    } 
                }
            }                
                
            // switch to this mode immediately
            switch(AnimationData.eMode) {
                case 0:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break;
                case 1:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode1; break;
                case 2:  gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode2; break;
                default: gtSysState.eModeOfWarningLight = E_MODE_OF_WARNING_LIGHT__Mode3; break; // shd not happen
            }
            LL_Helmet_ChangeStateTo_NextMode();
        }
    } else if((11) == length) { 
        if( 0 == memcmp("NETID RESET", p_data, 11) ) { // reset network ID
            extern T_LL_HardwareID gtHwID;
            memcpy(&(gtPara.netID), &(gtHwID.auc[0]), LL_ESB_BASE_ADDR_LEN); gtPara.netID_exist = 0; gulFlashStoreNeeded = 1;
            sprintf(string, "NETID reset\0");
						SendDataLen = strlen(string);
						ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);
        } else if (0 == memcmp("DEL REMOTES", p_data, 11)) { 
            gtPara.tRemote.bits_isCheapRemote = 0;
            for(int indexDevice = 0; indexDevice < DEVICE_NUM_OF_XXX; indexDevice++) {
                memset(gtPara.tRemote.HardwareID[indexDevice].auc, 0xFF, LL_HARDWARE_ID_LEN);
            }
            sprintf(string, "Remotes deleted.\0");
						SendDataLen = strlen(string);
						ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);                                                       
        }
    } else if(12 == length) {
        if(
            ('T' == p_data[0]) && 
            ('G' == p_data[1]) &&
            ('W' == p_data[2]) &&
            ('T' == p_data[3]) &&
            ('H' == p_data[4])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) { 
                    sprintf(string, "FGWTH\0");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    gtPara.ssThreshhold = atoi((const char *)(&(p_data[5]))); 
                    gulFlashStoreNeeded = 1;
                }
#endif                
        }  
    } else if(16 == length) {            
        if(
            ('T' == p_data[0]) && 
            ('G' == p_data[1]) &&
            ('W' == p_data[2])    ) {
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) { 
                    sprintf(string, "FGW\0");
										SendDataLen = strlen(string);
										ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle);  

                    gtPara.ssKxyz[0] = atoi((const char *)(&(p_data[ 3]))); 
                    gtPara.ssKxyz[1] = atoi((const char *)(&(p_data[ 7]))); 
                    gtPara.ssKxyz[2] = atoi((const char *)(&(p_data[11]))); 
                    gulFlashStoreNeeded = 1;
                }
        }
    } else if(18 == length) {
        if( ('T' == p_data[0]) && 
            ('W' == p_data[1]) &&
            ('S' == p_data[2]) &&
            ('N' == p_data[3])    ) {
#ifndef _LL_DELETE_PRODUCTION_CODE
                if(SYS_PRODUCTION_TESTING_MODE == gtSysState.eOnOFF) {
										sprintf(string, "FWSN\0");
                    SendDataLen = strlen(string);
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
                    memcpy(&(gtPara.aucSN[0]), &(p_data[4]), 14);
                    gulFlashStoreNeeded = 1;                    
                    // turn off the LED, then enter pairing mode:
                    LL_LEDs_OFF(); //LL_Helmet_RedGreenFlashWhenTestMode_OFF();
                    gulCmdTestingMode = 1; LL_Helmet_ChangeStateTo_Pairing();            
                }
#endif                
        } 
    }
    sgulNusDataHandleNeeded = 0;
}	

#if 0
    if (0 == memcmp("ANIMATION_FRONT", p_data, 18)) { 
        gtPara.eModeOfWarningLight = p_data[0];  //which mode
        gtPara.ePatternOfFrontLight = p_data[1];  // which animation type
        gtPara.ulFrontFrameSpeedLevelAndColorType[gtPara.ePatternOfFrontLight] = (p_data[2] << 24 || p_data[3] << 16 || p_data[4] << 8 || p_data[5]); 
        
        gtPanelPara.gulWs2812FrameFrontSpeedLevel =  p_data[2]; //调速接口
                                                                //颜色接口

        gulFlashStoreNeeded = 1;
#endif
// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    if(1 == gulDisconnectRountine) { // Xiong: prevent DFU from a non-pairing phone.
        event = BLE_DFU_EVT_BOOTLOADER_NOT_ENTER;
    }
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
 
            // Prevent device from advertising on disconnect.
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
            config.ble_adv_on_disconnect_disabled = true;
            ble_advertising_modes_config_set(&m_advertising, &config);
 
            // Disconnect all other bonded devices that currently are connected.
            // This is required to receive a service changed indication
            // on bootup after a successful (or aborted) Device Firmware Update.
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }
 
        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;
 
        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;
 
        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;
 
        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}

/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;

    ble_dis_init_t     dis_init;
    ble_bas_init_t 	   bas_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
		
    // Initialize DFU.
    ble_dfu_buttonless_init_t dfus_init = {0};
    
    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service
    memset(&dis_init, 0, sizeof(dis_init));
		
    ble_srv_ascii_to_utf8( &dis_init.manufact_name_str , (char *)MANUFACTURER_NAME );
    ble_srv_ascii_to_utf8( &dis_init.fw_rev_str        , (char *)LL_VER_BLE_DIS    );
		
	dis_init.dis_char_rd_sec = SEC_OPEN;
		
	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
		
	// Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;
		
    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;
    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE:
            ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
                    
            if(SYS_PAIRING == gtSysState.eOnOFF) { // pairing mode
                //SEGGER_RTT_WriteString(0, "pairing: connected. \n");
            } else {
                gulDisconnectRountine = 1; // launch dis
                gulDisconnectCnt = gulTimerCnt1ms;
            } 
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for updating advertising.
 */
void LL_BLE_SetAdManu(unsigned short manu)
{
    ret_code_t err_code;   
    static unsigned char spec_data[4];

    ble_advdata_manuf_data_t manufacturer_data;

    
    ble_advdata_t adv;

    memset(&scanrsp, 0, sizeof(scanrsp));

    memset(&adv, 0, sizeof(adv));

    adv.name_type = BLE_ADVDATA_FULL_NAME;
    adv.include_appearance = true;
    adv.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // 4 bytes version info
    spec_data[0] = LL_VER_H;
    spec_data[1] = LL_VER_L;
    spec_data[2] = (unsigned char) ((LL_VER_DATE & 0xFF00) >> 8);
    spec_data[3] = (unsigned char) ((LL_VER_DATE & 0x00FF) >> 0);

    manufacturer_data.company_identifier = manu;
    manufacturer_data.data.p_data = spec_data;
    manufacturer_data.data.size   = sizeof(spec_data);
			
    scanrsp.p_manuf_specific_data = &manufacturer_data;

    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;
		
    //ble_advdata_set(0, &scanrsp); 
    err_code = ble_advertising_advdata_update(&m_advertising, &adv, &scanrsp);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    *p_erase_bonds = false;
}

void LL_Power_BeforeSleep(void)
{
    if(gulFlashStoreNeeded == 1 || gulFlashStoreNeeded == 2){ //If you have data to store before sleep
        if(flash_store_finish()){
            flash_store_beforeSleep();
        }
    }
    LL_GPIO_OutputWrite(0, LL_PIN_CHARGING_POWER, LL_PIN_N__CHARGING_POWER);
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_N__LED_POWER_ENABLE); 
    LL_GPIO_InputCfg(0, LL_PIN__ADAPTER_BOARD_DETECT, LL_GPIO_PULL_NONE, LL_GPIO_TRIGGER_NONE);
    LL_GPIO_OutputWrite(0, LL_PIN__BATT_LVL, LL_PIN_N__BATT_LVL); // turn off the ADC cierr_codeuit.//LL_GPIO_OutputWrite(0, LL_PIN_BATT_LVL, LL_NORMAL_BATT_LVL); // turn off the ADC cierr_codeuit.
    LL_LEDs_OFF();
    
    //wait flash write success 
    if(!flash_store_finish()){
        for(uint16_t i=0; i<LL_FLASH_WAITING_TIME_BEFORE_SLEEP; i++){
            if(flash_store_finish()){break;}
            nrf_delay_ms(1);
        }
    }
    LL_PWM_Sleep();LL_Key_Init();
}

void LL_Power_Sleep(void)
{
    sd_power_system_off();
}

#if 1 // a function for "send flash mode to app"
void TxCurrentFlashModeToApp__Start(void) ;
unsigned long TxFlashModeParaToApp__time_cnt = 0;
unsigned long TxFlashModeParaToApp__step_cnt = 0;
void TxFlashModeParaToApp__Start(void) { 
    LL_Timer_CntStart(TxFlashModeParaToApp__time_cnt);
    TxFlashModeParaToApp__step_cnt = 0; 
}

void TxFlashModeParaToApp__Stop( void) { 
    LL_Timer_CntStop(TxFlashModeParaToApp__time_cnt);
//  TxFlashModeParaToApp__step_cnt = 0; 
}
void TxFlashModeParaToApp(void) { 
    if(0 == TxFlashModeParaToApp__time_cnt) { // this func is OFF
        return; 
    } else { 
        if(50 > LL_Timer_Elapsed_ms(TxFlashModeParaToApp__time_cnt)) { // not time yet
            return;
        } else { // timeout, then reset timer cnt
            LL_Timer_CntStart(TxFlashModeParaToApp__time_cnt);
        }
    }
    unsigned char tx_buf[16]; uint16_t tx_len = 0;
    unsigned long mode, part, e_LED, animationType;
    unsigned char brightness, color;
    // send diff data according to the step
    switch(TxFlashModeParaToApp__step_cnt) {
        case 0: mode = 0; part = LL_PART_FRONT_LIGHT; if(gtPara.ePatternOfFrontLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){ animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_FRONT;     break;
        case 1: mode = 0; part = LL_PART_BACK_LIGHT;  if(gtPara.ePatternOfRearLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){  animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_REAR;      break;
        case 2: mode = 1; part = LL_PART_FRONT_LIGHT; if(gtPara.ePatternOfFrontLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){ animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_FRONT;     break;
        case 3: mode = 1; part = LL_PART_BACK_LIGHT;  if(gtPara.ePatternOfRearLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){  animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_REAR;      break;
        case 4: mode = 2; part = LL_PART_FRONT_LIGHT; if(gtPara.ePatternOfFrontLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){ animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_FRONT;     break;
        case 5: mode = 2; part = LL_PART_BACK_LIGHT;  if(gtPara.ePatternOfRearLight[mode] <= LED_ANIMATION_FASTFLASH_MODE){  animationType = LL_TYPE_COMMON_ANIMATION;}else{ animationType = gtPara.ePatternOfFrontLight[mode]-2;}  e_LED = E_LL_LED_REAR;      break;
        case 6: mode = 0; part = 0xFFFFFFFF; break;
        case 7: mode = 1; part = 0xFFFFFFFF; break;
        case 8: mode = 2; part = 0xFFFFFFFF; break;        
        case 9: mode = 0; part = 0xFFFFFFFF; break;
        case 10: mode = 1; part = 0xFFFFFFFF; break;
        case 11: mode = 2; part = 0xFFFFFFFF; break;    
        case 12: part = 0xAAAAAAA; break;
        default: TxFlashModeParaToApp__Stop(); return;
    }

    if(0xFFFFFFFF==part) { // brightness
        tx_buf[0] = 'B';
        tx_buf[1] = 'M';
        tx_buf[2] = mode;
        if(TxFlashModeParaToApp__step_cnt < 9){
            //FRONT
            tx_buf[3] = LL_PART_FRONT_LIGHT;
            LL_Find_Color_Position(gtPara.brightness_individual[mode][E_LL_LED_FRONT], &color, &brightness);  
            tx_buf[4] = color ; 
            tx_buf[5] = brightness;
        }else{
            //REAR
            tx_buf[3] = LL_PART_BACK_LIGHT;
            LL_Find_Color_Position(gtPara.brightness_individual[mode][E_LL_LED_REAR], &color, &brightness); 
            tx_buf[4] = color ; 
            tx_buf[5] = brightness;
        }            
        tx_len = 6;
    } else if(0xAAAAAAA==part) { // animation current mode
        //
        TxCurrentFlashModeToApp__Start();
    } else {       
        packFlashModeParaToApp(tx_buf, mode, part, animationType, LL_Para__get_flashing_pattern(mode, (E_LL_LED)e_LED));//gtPara.customer_flashing_timeslot_states[mode][e_LED]);
        tx_len = 6;
    }
    // wait
    if(NRF_SUCCESS == ble_nus_data_send(&m_nus, (uint8_t *)tx_buf, &tx_len, m_conn_handle)) { 
        TxFlashModeParaToApp__step_cnt++; 
    } else { /* need send again, so do not go to next step */ }
}
#endif

#if 1 // a function for "send current flash mode to app"
unsigned long TxCurrentFlashModeToApp__time_cnt = 0;
unsigned long TxCurrentFlashModeToApp__step_cnt = 0;
void TxCurrentFlashModeToApp__Start(void) { 
        unsigned char tx_buf[5];
        uint16_t tx_len = 5;
        tx_buf[0] = 'f';
        tx_buf[1] = '0' + gtPara.eModeOfWarningLight;
        tx_buf[2] = 'f';
        tx_buf[3] = 'f';
        tx_buf[4] = 'f';
        ble_nus_data_send(&m_nus, (uint8_t *)tx_buf, &tx_len, m_conn_handle);
}
void TxCurrentFlashModeToApp__Stop( void) { 
    LL_Timer_CntStop(TxCurrentFlashModeToApp__time_cnt);
//  TxCurrentFlashModeToApp__step_cnt = 0; 
}
void TxCurrentFlashModeToApp(void) { 
    if(0 == TxCurrentFlashModeToApp__time_cnt) { // this func is OFF
        return; 
    } else { 
        if(50 > LL_Timer_Elapsed_ms(TxCurrentFlashModeToApp__time_cnt)) { // not time yet
            return;
        } else { // timeout, then reset timer cnt
            LL_Timer_CntStart(TxCurrentFlashModeToApp__time_cnt);
        }
    }
    switch(TxCurrentFlashModeToApp__step_cnt) {
        case 0: break;
        default: TxCurrentFlashModeToApp__Stop(); return;
    }
    unsigned char tx_buf[5];
    uint16_t tx_len = 5;
//    tx_buf[0] = 'f';
//    tx_buf[1] = '0' + gtPara.eMode;
    packCurrentModeToApp(tx_buf, gtPara.eModeOfWarningLight);
//    ble_nus_string_send(&m_nus, tx_buf, 5);
    if(NRF_SUCCESS == ble_nus_data_send(&m_nus, (uint8_t *)tx_buf, &tx_len, m_conn_handle)) { 
        TxCurrentFlashModeToApp__step_cnt++; 
    } else { /* need send again, so do not go to next step */ }    
}
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

#if 0
/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}
#endif

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

unsigned char glAdapterBoardConnState = 0;  // 1:Connect 2:Disconnect 
//static unsigned long timestampReportBatteryToApp;
unsigned long MarkChangeAddrTime;
unsigned long ulPoweronDisplayTime;
/**@brief Application main function.
 */
int main(void)
{
    bool erase_bonds;
    ret_code_t err_code;
    uint32_t glAdapterBoardConCnt = 0, glAdapterBoardDisconCnt = 0;
    uint16_t SendDataLen;
    
    LL_HardwareID_Get(&gtHwID);       
    // Initialize the async SVCI interface to bootloader before any interrupts are enabled.
    err_code = ble_dfu_buttonless_async_svci_init();
    APP_ERROR_CHECK(err_code);
    // Initialize.

    timers_init();
    buttons_leds_init(&erase_bonds);
    //power_management_init();
    ble_stack_init();
    
    LL_BLE_Observer__Init();
    
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

#ifdef LL_ESB_TIMESLOT
    err_code = esb_timeslot_init();
    APP_ERROR_CHECK(err_code);
	
    err_code = esb_timeslot_sd_start();
    APP_ERROR_CHECK(err_code);
#endif
    // Start execution.
    advertising_start();
    
	LL_Helmet_ChangeStateTo_Init();
    LL_Key_Init();

	flash_init();
	flash_load();
    
    LL_PWM_Init(LL_HW_PWM_PERIOD);
    
    LL_LEDs_init();
    
    LL_Battery_Charging__Init();
    LL_ADC_Init();
    LL_Battery_Init();

    LL_GPIO_OutputCfg(0, LL_PIN_LED_POWER_ENABLE, LL_PULLMODE_LED_POWER_ENABLE, LL_PIN_N__LED_POWER_ENABLE);    

    LL_GPIO_OutputCfg(0, LL_PIN_CHARGING_POWER, LL_PULLMODE_CHARGING_POWER, 0);     
    
    LL_GPIO_InputCfg(0, LL_PIN__ADAPTER_BOARD_DETECT, LL_PULL__ADAPTER_BOARD_DETECT, LL_GPIO_TRIGGER_NONE);

    LL_GPIO_InputCfg(0, LL_PIN__TESTING_MODE_PIN0, LL_PULL__TESTING_MODE, LL_GPIO_TRIGGER_NONE);
    LL_GPIO_InputCfg(0, LL_PIN__TESTING_MODE_PIN1, LL_PULL__TESTING_MODE, LL_GPIO_TRIGGER_NONE);  
    
    if(   (LL_PIN_Y__TESTING_MODE == LL_GPIO_InputRead(0, LL_PIN__TESTING_MODE_PIN0))
       || (LL_PIN_Y__TESTING_MODE == LL_GPIO_InputRead(0, LL_PIN__TESTING_MODE_PIN1)) ) {
           LL_Helmet_ChangeStateTo_PcbTestingMode();
   }else if(LL_Battery__isCharging()) { // if charging
        // unfreeze the power-off-if-low-battery.
        void LL_Charging_Display(void);
        LL_Charging_Display();
        //#include "LL_Charging_Display.h"    
    }else if( ( gatKeyCfg[LL_KEY_NUM_ONOFF  ].normal != LL_GPIO_InputRead(gatKeyCfg[LL_KEY_NUM_ONOFF  ].port, gatKeyCfg[LL_KEY_NUM_ONOFF  ].pin) ) ) { // if key pressed
        LL_Key_Init_With_No_Trigger();
        //continue 
    }else{
        LL_Power_BeforeSleep(); 
        LL_Power_Sleep();   
	}

    //NRF_LOG_INFO("Debug logging for UART over RTT started.");

    char string[16];
    unsigned long ulTimerCntPwm = gulTimerCnt1ms; 
    unsigned long ulTimerCntBatteryUpdate = gulTimerCnt1ms;
    unsigned long ulTimerCntAdapterBoardDetect = gulTimerCnt1ms;
    MarkChangeAddrTime = gulTimerCnt1ms;
    ulPoweronDisplayTime = gulTimerCnt1ms;
    // Enter main loop.
    for (;;)
    {
        
        LL_Key_Scan();
     
		//battery
        if(1 == gulFlashStoreNeeded) { gulFlashStoreCnt = gulTimerCnt1ms;
            gulFlashStoreNeeded = 2;                          
        }else if(gulFlashStoreNeeded == 2){
            if(2000 <= LL_Timer_Elapsed_ms(gulFlashStoreCnt)) { 
                if(flash_store_finish()){
                    gulFlashStoreCnt = gulTimerCnt1ms;
                    flash_store();gulFlashStoreNeeded = 0;
                }
            }  
        }
        LL_Battery_Mainloop();

        if(4 <= LL_Timer_Elapsed_ms(ulTimerCntPwm)) { ulTimerCntPwm = gulTimerCnt1ms;
            LL_PWM_perDuty(); // 4ms per duty
        }
        if(1000 <= LL_Timer_Elapsed_ms(ulTimerCntAdapterBoardDetect)) {  //10S JUDGE
            ulTimerCntAdapterBoardDetect = gulTimerCnt1ms;
            if(LL_GPIO_InputRead(0, LL_PIN__ADAPTER_BOARD_DETECT) == LL_PIN_Y__ADAPTER_BOARD_DETECT){  //plug

                glAdapterBoardConCnt++; glAdapterBoardDisconCnt = 0;
                if(glAdapterBoardConCnt== 10){glAdapterBoardConnState = ADAPTEBOARD_CONNECT;} 
            }else{ 
                glAdapterBoardDisconCnt++; glAdapterBoardConCnt = 0;  
                if(glAdapterBoardDisconCnt== 10){glAdapterBoardConnState = ADAPTEBOARD_DISCONNECT;} 
            }
        }

        if(4000 <= LL_Timer_Elapsed_ms(ulTimerCntBatteryUpdate)) { ulTimerCntBatteryUpdate = gulTimerCnt1ms;
            // battery level of helmet
            battery_level_update();
            // battery level of remote
            if(    (1 == gulRemoteBatteryAutoUpdate) 
                && (SYS_PCB_TESTING_MODE != gtSysState.eOnOFF) 
                && (SYS_OFF != gtSysState.eOnOFF) 
                && (0 == gulCmdTestingMode)
                && (SYS_PRODUCTION_TESTING_MODE   != gtSysState.eOnOFF) ) {
                    if(0xCCCCCCCC == gulBatteryOfRemote) { // if Remote Lite
                            sprintf(string, "BL-R:Lite\0");
                    } else { // else, Lumos Remote
                            if(0xFFFFFFFF != gulBatteryOfRemote) { sprintf(string, "BL-R:%03u\0", gulBatteryOfRemote); }
                            else                                 { sprintf(string, "BL-R:NA\0");                    }
                    }
                    uint16_t ulLen = strlen(string); //if(strlen("BL-R:100") > ulLen) { ulLen = strlen("BL-R:100"); }
                    ble_nus_data_send(&m_nus, (uint8_t *)string, &ulLen, m_conn_handle);  
                }
        }
       
       LL_BatteryIndicator__Mainloop();

       LL_LEDs_routine();  
        
       LL_Helmet_Mainloop();	      
        if( 1 == gulMsgHandleNeeded     ) { 
            if( (55 > gulRSSI) || (SYS_PAIRING != gtSysState.eOnOFF) ) {
                LL_MsgHandle_Helmet(&sgtMsg); 
                if(1==gulCmdTestingMode) { gulCmdTestingMode = 2; } 
            }
            gulMsgHandleNeeded = 0; 
                        //char string[32]; sprintf(string, "RSSI: %u \n", gulRSSI);
                        //SEGGER_RTT_WriteString(0, string);
        }        
       nus_data_handler_real();

        if(LL_ESB_data_received) {
            nrf_esb_payload_t rx_payload;
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) { 
                if(0 == rx_payload.pipe) {
                    if(rx_payload.length == sizeof(T_MSG_toHelmet)) { 
                        gulRSSI = rx_payload.rssi;
                        //
                        memcpy(&sgtMsg, rx_payload.data, sizeof(T_MSG_toHelmet));
                        gulMsgHandleNeeded = 1;
                        #ifdef _LL_2p4GHz_RELAY
                            flag_for_relay_remote_msg = 1; time_for_relay_remote_msg = gulTimerCnt1ms; // relay it to others
                        #endif                        //
                        gtSysState.ulNoConnectionSinceSysOn = 0; // 0: mark that "some remote has connected"
                    } else if(rx_payload.length == sizeof(T_BROADCAST_MSG)) {
                        T_BROADCAST_MSG eBroadcastMsg;
                        memcpy(&eBroadcastMsg, rx_payload.data, sizeof(T_BROADCAST_MSG));
                        memcpy( &(gtPara.tRemote.HardwareID[0]), &(eBroadcastMsg.pairedRemoteID[0]), sizeof(T_LL_HardwareID) ); gulFlashStoreNeeded = 1;
                        memcpy( &(gtPara.tRemote.HardwareID[1]), &(eBroadcastMsg.pairedRemoteID[1]), sizeof(T_LL_HardwareID) ); gulFlashStoreNeeded = 1;
										}	else if(rx_payload.length == sizeof(T_BROADCAST_MSG_POWER_OFF)) { 
                        if(1 == getPara(syncPowerOff)) {    
                            LL_LEDs_OFF();
														LL_Helmet_ChangeStateTo_BeepBeforeOFF();   
                        }
                    } else if( (rx_payload.length == sizeof(T_BROADCAST_MSG_PUBLIC            ))
                           ||  (rx_payload.length == sizeof(T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT)) ) { 
                                geBroadcastState = E_BROADCAST_STATE__OFF;
                                LL_MSG_Handle__PublicMessage_saveWhenInInterrupt(rx_payload.data, rx_payload.length);
                    }else { /* noise */ }
                } else if(1 == rx_payload.pipe) { 
                    if( (rx_payload.length == sizeof(T_BROADCAST_MSG_PUBLIC            ))
                    ||  (rx_payload.length == sizeof(T_BROADCAST_MSG_PUBLIC_RE_TRANSMIT)) ) { 
                        geBroadcastState = E_BROADCAST_STATE__OFF; 
                        LL_MSG_Handle__PublicMessage_saveWhenInInterrupt(rx_payload.data, rx_payload.length);
                    } else { /* noise */ }
                }
            }            
        LL_ESB_data_received = 0; }			
		
        TxFlashModeParaToApp();
        LL_MSG_Handle__PublicMessage_parseWhenInMainloop();  

        if( (1 == gulDisconnectRountine) && (SYS_PCB_TESTING_MODE != gtSysState.eOnOFF) ) {
            // check timeout:
            if(3*1000 <= LL_Timer_Elapsed_ms(gulDisconnectCnt)) {
                sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                gulDisconnectRountine = 0;
            }
        }
        // cmd testing mode:
        static unsigned char sgaucFinalTestPack[20];
        if(0 == gulCmdTestingMode) { 
            // likely
        } else if(1 == gulCmdTestingMode) {
            // wait pairing.
        } else if(2 == gulCmdTestingMode) {
            // pack
            memcpy(&sgaucFinalTestPack[0], sgtMsg.tHardwareID.auc, 8);
            sgaucFinalTestPack[8] = (unsigned char)gulBatteryOfRemote;
            // next step
            gulCmdTestingMode = 3;
        } else if(3 == gulCmdTestingMode) {
            SendDataLen = 9;
            ble_nus_data_send(&m_nus, (uint8_t *)string, &SendDataLen, m_conn_handle); 
            // next step
            gulCmdTestingMode = 4;          
        } else if(4 == gulCmdTestingMode) {
            /* wait TSHUT or TPAIR.  */
//      } else {
//          if( 3000 < LL_Timer_Elapsed_ms(gulCmdTestingMode) ) {
                // suto sleep
//              LL_Power_BeforeSleep(); 
//              LL_Power_Sleep(); 
//          }
        }        
    }
}

void LL_ESB_callback_esb_started(void) {
    LL_Sync();
}
/**
 * @}
 */

#include "LL_LED_Panel_Data_CommonFlash.h"
void LL_Charging_Display(void)
{

    unsigned long ulSample_1,ulSample_2;
    unsigned char glBatRank;
    unsigned char sgulChargingDisplayStep = 0,sgulChargingGeneralStep = 0;

    unsigned long ulTimerFreqUpdateAnimationCnt = 1800;

    extern bool glPowerOnAnimationPlayOff;
    glPowerOnAnimationPlayOff = true; //don't need play at first
    LL_GPIO_OutputWrite(0, LL_PIN_CHARGING_POWER, LL_PIN_Y__CHARGING_POWER); 
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_Y__LED_POWER_ENABLE);
    T_LL_KEY_EVT tKeyEvt;
    LL_Key_Init_With_No_Trigger();
    
    (void) sd_ble_gap_adv_stop(m_advertising.adv_handle);
    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    LL_BLE_Observer__Stop();
    //Front 
    memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));
    
    //Charging Animation
    LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARGING);
    unsigned long ulTimerCntBatteryUpdateWhenCharging = gulTimerCnt1ms;
    while(1) {
        
        LL_Battery_Mainloop(); 
        LL_Key_Scan();
            
        switch(sgulChargingGeneralStep) {
            case  0:
                // if ON/OFF key pressed: check whether a short or long press.
                if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (1 == tKeyEvt.evt) ) {
                    sgulChargingGeneralStep++; 
                }
                break;    
            case  1:
                // if ON/OFF key released: ON!
                if( LL_Key_EvtFetch(&tKeyEvt) && (LL_KEY_NUM_ONOFF == tKeyEvt.N) && (0 == tKeyEvt.evt) ) {
                    sgulChargingGeneralStep = 0;
                    if(sgulChargingDisplayStep == 0){
                        sgulChargingDisplayStep = 1;
                    }else if(sgulChargingDisplayStep == 1){
                        sgulChargingDisplayStep = 0;
                    }
                }
                break;    
        }
        
        if(ulTimerFreqUpdateAnimationCnt <= LL_Timer_Elapsed_ms(ulTimerCntBatteryUpdateWhenCharging)) { ulTimerCntBatteryUpdateWhenCharging = gulTimerCnt1ms;
            switch(sgulChargingDisplayStep)
            {
                case 0:
                    //charging low level && standby high level  mean charing 
                    //charging high level && standby low level  mean full-battery 
                    //charging high level && standby high level  mean warning!
                
                    //charging 
                    ulSample_1 = VOLTAGE_OF_AD_VALUE(LL_ADC_Sample_1);
                    //charging standby
                    ulSample_2 = VOLTAGE_OF_AD_VALUE(LL_ADC_Sample_2);

                    if(ulSample_1 > 1000 && ulSample_2 < 100){glBatRank = 0xFF;} 
                    //}else if(ulSample_1 < 100 && ulSample_2 < 100){
                    ////warning
                    //}else if(ulSample_1 < 100 && ulSample_2 > 1000){ //charging
                    else{   // don't need to handle warning now    
                        //battery
                        if(glBatRank != 0xFF){glBatRank = LL_Battery_Level()/20;}
                        //if(glBatRank > gtPara.glBatteryRank){ //battery up,need store
                        //    gtPara.glBatteryRank = glBatRank;
                        //    gulFlashStoreNeeded = 1;
                        //}
                        if(gtPara.ulNeedCharge){
                            if(glBatRank >= 1){gtPara.ulNeedCharge = 0; gulFlashStoreNeeded = 1;}
                        }
                    }
                    break;
                case 1:
                    //turn off battery display;
                    break;
            }
            
            if(gulFlashStoreNeeded == 1){
                flash_store();
                gulFlashStoreNeeded = 0;
            }
            
            //Rear
            memset(gtPanelPara.animationFramesRear_flash, 0,sizeof(gtPanelPara.animationFramesRear_flash));            
            
            if(sgulChargingDisplayStep == 0){                 
                if(glBatRank >= 5){  LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_5); }
                else if(glBatRank == 4){   LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_4); }
                else if(glBatRank == 3){   LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_3); }
                else if(glBatRank == 2){   LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_2); }
                else if(glBatRank == 1){   LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_1); }
                else if(glBatRank == 0){   LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARING_LEVEL_0); }
            }else{/*don't show the battery display*/}                
            
            if(!LL_Battery__isCharging()) { 
                gulFlashStoreNeeded = 1;
                LL_Power_BeforeSleep(); 
                LL_Power_Sleep();             
            }
        }
        LL_LEDs_routine();  
       
    }      
}  
