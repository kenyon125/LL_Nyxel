#include "esb_timeslot.h"

#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
//#include "softdevice_handler.h"
#include "boards.h"
#include "sdk_common.h"
#include "app_util_platform.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_sdh_soc.h"

#include "LL_Clock.h"
#include "LL_LED_Helmet.h"
#include "LL_MSG.h"
#include "LL_MSG_Handle_Helmet.h"
#include "LL_Para.h"
#include "LL_SysMode.h"
#include "LL_Clock.h"
#include "LL_Timer.h"
#define TIMESLOT_BEGIN_IRQn         COMP_LPCOMP_IRQn             /**< Re-used LPCOMP interrupt for processing the beginning of timeslot. */
#define TIMESLOT_BEGIN_IRQHandler   COMP_LPCOMP_IRQHandler       /**< The IRQ handler of LPCOMP interrupt */
#define TIMESLOT_BEGIN_IRQPriority  3                       /**< Interrupt priority of @ref TIMESLOT_BEGIN_IRQn. */

#define TIMESLOT_END_IRQn           QDEC_IRQn               /**< Re-used QDEC interrupt for processing the end of timeslot. */
#define TIMESLOT_END_IRQHandler     QDEC_IRQHandler         /**< The IRQ handler of QDEC interrupt */
#define TIMESLOT_END_IRQPriority    3                       /**< Interrupt priority of @ref TIMESLOT_END_IRQn. */

#define MAX_TX_ATTEMPTS             10                      /**< Maximum attempt before discarding the packet (the number of trial = MAX_TX_ATTEMPTS x retransmit_count, if timeslot is large enough) */
#define TS_LEN_US                   (90000UL)                /**< Length of timeslot to be requested. */
#define TX_LEN_EXTENSION_US         (5000UL)                /**< Length of timeslot to be extended. */
#define TS_SAFETY_MARGIN_US         (700UL)                 /**< The timeslot activity should be finished with this much to spare. */
#define TS_EXTEND_MARGIN_US         (2000UL)                /**< Margin reserved for extension processing. */


static volatile enum
{
    LL_ESB_STATE_IDLE,      /**< Default state */
    LL_ESB_STATE_RX,        /**< Waiting for packets */
    LL_ESB_STATE_TX         /**< Trying to transmit packet */
} m_state = LL_ESB_STATE_IDLE;


/** Constants for timeslot API */
static nrf_radio_request_t          m_timeslot_request;     /**< Persistent request structure for softdevice. */
static nrf_esb_config_t             nrf_esb_config = NRF_ESB_LEGACY_CONFIG;         /**< Configuration structure for nrf_esb initialization. */
//static ut_data_handler_t            m_evt_handler = 0;      /**< Event handler which passes received data to application. */

static nrf_radio_signal_callback_return_param_t signal_callback_return_param;   /**< Return parameter structure to timeslot callback. */
static uint32_t                     m_total_timeslot_length = 0;                /**< Timeslot length. */
void RADIO_IRQHandler(void);


/** Address. */
static const uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };    /**< Address prefix. */


/**@brief Request next timeslot event in earliest configuration.
 * @note  Will call softdevice API.
 */
uint32_t request_next_event_earliest(void)
{
    m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
    m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_NORMAL;
    m_timeslot_request.params.earliest.length_us   = LL_TIMESLOT_LENGTH;
    m_timeslot_request.params.earliest.timeout_us  = LL_TIMESLOT_TIMEOUT;
    return sd_radio_request(&m_timeslot_request);
}


/**@brief Configure next timeslot event in earliest configuration.
 */
void configure_next_event_earliest(void)
{
    m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
    m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_NORMAL;
    m_timeslot_request.params.earliest.length_us   = LL_TIMESLOT_LENGTH;
    m_timeslot_request.params.earliest.timeout_us  = LL_TIMESLOT_TIMEOUT;
}


/**@brief Configure next timeslot event in normal configuration.
 */
 /*
void configure_next_event_normal(void)
{
    m_timeslot_request.request_type               = NRF_RADIO_REQ_TYPE_NORMAL;
    m_timeslot_request.params.normal.hfclk        = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
    m_timeslot_request.params.normal.priority     = NRF_RADIO_PRIORITY_HIGH;
    m_timeslot_request.params.normal.length_us    = TS_LEN_US;
    m_timeslot_request.params.normal.distance_us  = 100000;
}*/


/**@brief Timeslot signal handler.
 */
void nrf_evt_signal_handler(uint32_t evt_id, void* context)
{
    uint32_t err_code;

    switch (evt_id)
    {
        case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
            // No implementation needed
            break;

        case NRF_EVT_RADIO_SESSION_IDLE:
            err_code = sd_radio_session_close();
            APP_ERROR_CHECK(err_code);
            break;

        case NRF_EVT_RADIO_SESSION_CLOSED:
            // No implementation needed, session ended
            break;

        case NRF_EVT_RADIO_BLOCKED:
            // Fall through
    
        case NRF_EVT_RADIO_CANCELED:
            err_code = request_next_event_earliest();
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

NRF_SDH_SOC_OBSERVER(m_esb_evt_observer, 0, nrf_evt_signal_handler, NULL);
/**@brief Timeslot event handler.
 */
nrf_radio_signal_callback_return_param_t * radio_callback(uint8_t signal_type)
{
    //Initialize with default action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE:
    signal_callback_return_param.params.request.p_next = NULL;
    signal_callback_return_param.callback_action       = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
    switch(signal_type)
    {
        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
            /* Start of the timeslot - set up timer interrupt */
            //nrf_gpio_pin_toggle(28);	
            NRF_TIMER0->TASKS_STOP          = 1;
            NRF_TIMER0->TASKS_CLEAR         = 1;
            NRF_TIMER0->MODE                = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
            NRF_TIMER0->EVENTS_COMPARE[0]   = 0;
            NRF_TIMER0->EVENTS_COMPARE[1]   = 0;
            NRF_TIMER0->INTENSET            = TIMER_INTENSET_COMPARE0_Msk | TIMER_INTENSET_COMPARE1_Msk ;
            NRF_TIMER0->CC[0]               = LL_TIMESLOT_LENGTH - TS_SAFETY_MARGIN_US;
            NRF_TIMER0->CC[1]               = (LL_TIMESLOT_LENGTH - TS_EXTEND_MARGIN_US);
            NRF_TIMER0->BITMODE             = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);
            NRF_TIMER0->TASKS_START         = 1;
            // Disable and enable the Radio to reset the RADIO registers, needed from S1xx v8.x
						NRF_RADIO->POWER                = (RADIO_POWER_POWER_Enabled << RADIO_POWER_POWER_Pos);
            /* Call TIMESLOT_BEGIN_IRQHandler later. */
            NVIC_EnableIRQ(TIMER0_IRQn); 
            NVIC_SetPendingIRQ(TIMESLOT_BEGIN_IRQn);
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
            RADIO_IRQHandler();
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
            if (NRF_TIMER0->EVENTS_COMPARE[0] &&
                (NRF_TIMER0->INTENSET & (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENCLR_COMPARE0_Pos)))
            {
                NRF_TIMER0->TASKS_STOP        = 1;
                NRF_TIMER0->EVENTS_COMPARE[0] = 0;
                /* Workaround for issue that Softdevice doesn't reinitialize CC[1]&CC[2]*/
                NRF_TIMER0->INTENCLR =TIMER_INTENSET_COMPARE2_Msk|TIMER_INTENSET_COMPARE1_Msk;
                NRF_TIMER0->CC[1]=0;
                NRF_TIMER0->CC[2]=0;
                /* This is the "timeslot is about to end" timeout. */
                if (!nrf_esb_is_idle())
                {
                    NRF_RADIO->INTENCLR      = 0xFFFFFFFF;
                    NRF_RADIO->TASKS_DISABLE = 1;
                }
		//nrf_gpio_pin_toggle(29);	
                /* Schedule next timeslot. */
                configure_next_event_earliest();
                signal_callback_return_param.params.request.p_next = &m_timeslot_request;
                signal_callback_return_param.callback_action       = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
            }

            if (NRF_TIMER0->EVENTS_COMPARE[1] &&
                (NRF_TIMER0->INTENSET & (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENCLR_COMPARE1_Pos)))
            {
                //nrf_gpio_pin_toggle(30);	
                NRF_TIMER0->EVENTS_COMPARE[1] = 0;

                /* This is the "Time to extend timeslot" timeout. */
                if (m_total_timeslot_length < (128000000UL - 1UL - TX_LEN_EXTENSION_US))
                {
                    /* Request timeslot extension if total length does not exceed 128 seconds. */
                    signal_callback_return_param.params.extend.length_us = TX_LEN_EXTENSION_US;
                    signal_callback_return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND;
                }
                else
                {
                    /* Return with no action request. */
                
                }
            }
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
            //nrf_gpio_pin_toggle(31);	
            NRF_TIMER0->TASKS_STOP          = 1;
            NRF_TIMER0->EVENTS_COMPARE[0]   = 0;
            NRF_TIMER0->EVENTS_COMPARE[1]   = 0;
            NRF_TIMER0->CC[0]              += (TX_LEN_EXTENSION_US - 25);
            NRF_TIMER0->CC[1]              += (TX_LEN_EXTENSION_US - 25);
            NRF_TIMER0->TASKS_START         = 1;

            m_total_timeslot_length += TX_LEN_EXTENSION_US;
            //NVIC_SetPendingIRQ(TIMESLOT_BEGIN_IRQn);
        
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED:
            /* Tried scheduling a new timeslot, but failed. */
            /* Disabling UESB is done in a lower interrupt priority. */
            /* Call TIMESLOT_END_IRQHandler later. */
            NVIC_SetPendingIRQ(TIMESLOT_END_IRQn);
            //nrf_gpio_pin_toggle(29);	
            break;

        default:
            /* No implementation needed. */
            break;
    }
    //default return action is NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE
    return (&signal_callback_return_param);
}


uint32_t esb_timeslot_sd_start(void)
{
    uint32_t err_code;

    err_code = sd_radio_session_open(radio_callback);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = request_next_event_earliest();
    if (err_code != NRF_SUCCESS)
    {
        (void)sd_radio_session_close();
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t esb_timeslot_sd_stop(void)
{
    return sd_radio_session_close();
}


/**@brief IRQHandler used for execution context management. 
  *       Any available handler can be used as we're not using the associated hardware.
  *       This handler is used to stop and disable UESB.
  */
void TIMESLOT_END_IRQHandler(void)
{
    //uint32_t err_code;

    /* Timeslot is about to end: stop UESB. */
    nrf_esb_disable(); 
    m_state = LL_ESB_STATE_IDLE;
    m_total_timeslot_length = 0;

   
}
       
/**@brief IRQHandler used for execution context management. 
  *       Any available handler can be used as we're not using the associated hardware.
  *       This handler is used to initiate UESB RX/TX.
  */
void TIMESLOT_BEGIN_IRQHandler(void)
{
	LL_ESB_Start();
}


uint32_t esb_timeslot_init(void)
{
	void nrf_esb_event_handler(nrf_esb_evt_t const * p_event);
   // m_evt_handler = evt_handler;
	
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.selective_auto_ack       = true;
	//nrf_esb_config.retransmit_delay         = 250;
	//nrf_esb_config.payload_length           = 8;//6;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.crc                      = NRF_ESB_CRC_16BIT;
    nrf_esb_config.radio_irq_priority       = 0;
    nrf_esb_config.tx_output_power          = NRF_ESB_TX_POWER_4DBM;

    // Using three avilable interrupt handlers for interrupt level management
    // These can be any available IRQ as we're not using any of the hardware,
    // simply triggering them through software
    NVIC_ClearPendingIRQ(TIMESLOT_END_IRQn);
    NVIC_SetPriority(TIMESLOT_END_IRQn, 2);
    NVIC_EnableIRQ(TIMESLOT_END_IRQn);

    NVIC_ClearPendingIRQ(TIMESLOT_BEGIN_IRQn);
    NVIC_SetPriority(TIMESLOT_BEGIN_IRQn, 2);
    NVIC_EnableIRQ(TIMESLOT_BEGIN_IRQn);

    return NRF_SUCCESS;
}


//esb
void LL_ESB_Start(void) {
    LL_ESB_callback_esb_started();	
}

uint32_t esb_start_PRX(bool selective_auto_ack) { // need a return value due to "VERIFY_SUCCESS" did
    uint32_t err_code;

    if (m_state == LL_ESB_STATE_IDLE) {
        #if 1 // suggestion from Nordic forum to fix "wrong pattern": power cycle RADIO before init
            NRF_RADIO->POWER = 0;
            NRF_RADIO->POWER;
            NRF_RADIO->POWER = 1;
        #endif
        nrf_esb_config.mode               = NRF_ESB_MODE_PRX;
        nrf_esb_config.selective_auto_ack = selective_auto_ack;
        err_code = nrf_esb_init(&nrf_esb_config); APP_ERROR_CHECK(err_code);
        LL_ESB__set_address();
        err_code = nrf_esb_set_prefixes(addr_prefix, 8); APP_ERROR_CHECK(err_code);
    }
    nrf_esb_set_rf_channel(55);

    CRITICAL_REGION_ENTER();
    if (m_state != LL_ESB_STATE_RX)
    {
        /* No packets in the Tx FIFO: start reception */
        err_code = nrf_esb_start_rx();
        APP_ERROR_CHECK(err_code);
        m_state = LL_ESB_STATE_RX;
    }
    CRITICAL_REGION_EXIT();
    return NRF_SUCCESS;
}

uint32_t esb_start_PTX(bool selective_auto_ack) { // need a return value due to "VERIFY_SUCCESS" did
    uint32_t err_code;

    if (m_state == LL_ESB_STATE_IDLE) {
        #if 1 // suggestion from Nordic forum to fix "wrong pattern": power cycle RADIO before init
            NRF_RADIO->POWER = 0;
            NRF_RADIO->POWER;
            NRF_RADIO->POWER = 1;
        #endif
        nrf_esb_config.mode               = NRF_ESB_MODE_PTX;
        nrf_esb_config.selective_auto_ack = selective_auto_ack;
        err_code = nrf_esb_init(&nrf_esb_config); APP_ERROR_CHECK(err_code);
        LL_ESB__set_address();        
        err_code = nrf_esb_set_prefixes(addr_prefix, 8); APP_ERROR_CHECK(err_code);        
    }
    nrf_esb_set_rf_channel(55);
		
		CRITICAL_REGION_ENTER();
		if(m_state != LL_ESB_STATE_TX){
      if (m_state == LL_ESB_STATE_RX)
      {
        nrf_esb_stop_rx(); 
      }
      m_state = LL_ESB_STATE_TX;	
		}
	  CRITICAL_REGION_EXIT();
		
    return NRF_SUCCESS;
}


void LL_ESB_WriteTxBuf(unsigned char *pucData, unsigned long ulLen, unsigned long pipe, bool noack) {
    nrf_esb_payload_t tx_payload;
    memcpy(tx_payload.data, pucData, ulLen);
    tx_payload.length   = ulLen;
    tx_payload.pipe     = pipe;
    tx_payload.noack    = noack;//true;
    if (nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS) {
        //m_state = LL_ESB_STATE_TX;
    } else { 
    
			}
}

unsigned char LL_ESB_data_received = 0; 

//static nrf_esb_payload_t tx_payload;
/* PKChan: debug: static */ nrf_esb_payload_t rx_payload; 
extern T_MSG_toHelmet    sgtMsg;
T_MSG_toRemote gtMsgToRemote;
#if 1 // Xiong: ack-with-payload
unsigned long gulESB_NeedAckPayload = 0;
nrf_esb_payload_t tx_payload2 = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00);            
#endif            

//// multi ESB ID both when searching paired remotes when helmet on
//static const uint8_t base_addr_0_of_remotes[2][4];
//static const uint8_t base_addr_1_of_remotes[2][4];

extern unsigned long gulRSSI;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id) {
        case NRF_ESB_EVENT_TX_SUCCESS:
            //NRF_LOG("SUCCESS\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            //NRF_LOG("FAILED\r\n");
            (void) nrf_esb_flush_tx();
            m_state = LL_ESB_STATE_RX;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            LL_ESB_data_received = 1;
            break;
    }
    
}

unsigned char switch_base_addr_cnt = 0;
static const uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
static const uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};

#define COUNT_FOR_SWITCHING_ADDR    0  // 0: no switching; else (n): switch every n timeslots.
static unsigned char cnt_for_switching_addr = 0;
unsigned char ESB_address_type = 0; // 0: normal; 1: pairing

static uint32_t LL_ESB__set_address__Default(void) {
    uint32_t err_code;
    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);
    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);
    return NRF_SUCCESS;
}

extern T_LL_HardwareID gtHwID; // set address by nordic Hardware ID
static uint32_t LL_ESB__set_address__ForPairedRemotes(unsigned long indexPairedRemote) {
    uint32_t err_code;
    if(gtPara.tRemote.bits_isCheapRemote & (0x1<<indexPairedRemote)) { // is Cheap Remote
        // use the Cheap Remote's ID as ESB ID
        #if 1 // normal
            err_code = nrf_esb_set_base_address_0(&(gtPara.tRemote.HardwareID[indexPairedRemote].auc[0]));
        #else // for testing: always use Network ID for 2.4GHz module which means Remote Lite is ignored
            err_code = nrf_esb_set_base_address_0(&(gtPara.netID[0]));
        #endif
        VERIFY_SUCCESS(err_code);
        if(1 == gtPara.publicNetworkEnabled) { // if public sync is enabled
            err_code = nrf_esb_set_base_address_1(base_addr_1);//(&(gtPara.tRemote.HardwareID[switch_base_addr_cnt].auc[4]));
        } else {
            err_code = nrf_esb_set_base_address_1(&(gtPara.tRemote.HardwareID[indexPairedRemote].auc[4]));                        
        }
        VERIFY_SUCCESS(err_code);                
    } else { // not Cheap Remote
        // use the helmet's ID as ESB ID
        err_code = nrf_esb_set_base_address_0(&(gtPara.netID[0]));
        VERIFY_SUCCESS(err_code);
        if(1 == gtPara.publicNetworkEnabled) { // if public sync is enabled
            err_code = nrf_esb_set_base_address_1(base_addr_1);//(&(gtHwID.auc[4]));
        } else {
            err_code = nrf_esb_set_base_address_1(&(gtHwID.auc[4]));
        }
        VERIFY_SUCCESS(err_code);
    }    
    return NRF_SUCCESS;
}

extern unsigned long MarkChangeAddrTime;

uint32_t LL_ESB__set_address(void) {
//    uint32_t err_code;
    if(SYS_PAIRING == gtSysState.eOnOFF) { // if pairing mode
        LL_ESB__set_address__Default();
    } else { // else, normal mode
        cnt_for_switching_addr++; if( (COUNT_FOR_SWITCHING_ADDR > 0) && (cnt_for_switching_addr >= COUNT_FOR_SWITCHING_ADDR) ) { cnt_for_switching_addr = 0; // if need switching to default sometimes
            LL_ESB__set_address__Default();
        } else { // else, no need switching to default
            //
            // switching 2.4GHz address for all the paired remotes
            if (1 == gtSysState.ulNoConnectionSinceSysOn) { // no remote connected yet
                // switch ESB channel
                if(LL_ESB_data_received) { // if still waiting for handling
                    // DO NOT SWITCH
                } else { // else, no data need handle
									if(LL_Timer_Elapsed_ms(MarkChangeAddrTime) >= 500){
										switch_base_addr_cnt++; if(switch_base_addr_cnt >= 2) { switch_base_addr_cnt = 0;}
										MarkChangeAddrTime = gulTimerCnt1ms;
									}
                }
            } else { // some remote connected
 
            }
            //
            // set the address for one of the paired remotes
            if(gtPara.netID_exist) { // if has Network ID already
                uint32_t err_code;
                //
                // pipe 0
                if(1) { // always
                    err_code = nrf_esb_set_base_address_0(&(gtPara.netID[0]));
                }
                VERIFY_SUCCESS(err_code);
                //
                // pipe 1
                if(1 == gtPara.publicNetworkEnabled) { // if public sync is enabled
                    err_code = nrf_esb_set_base_address_1(base_addr_1);//(&(gtHwID.auc[4]));
                } else {
                    err_code = nrf_esb_set_base_address_1(&(gtHwID.auc[4]));
                }
                VERIFY_SUCCESS(err_code);
            } else { // else, no Network ID
                LL_ESB__set_address__ForPairedRemotes(switch_base_addr_cnt);
            }
        }
}
    return NRF_SUCCESS;
}
