/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/



// Usage:
//      (None)
//
// Remarks:
//      Refer to: SDK11 \examples\peripheral\twi_sensor\pca10028\arm5_no_packs
//      Set TWI0_ENABLED to 1 in "nrf_drv_config.h"!



#include "LL_Hardware_cfg.h"
#include "app_util_platform.h" // APP_IRQ_PRIORITY_HIGH
#include "nrf_drv_twi.h"       // twi



/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);

// flags of done
static unsigned long sgulTxDone = 1;
static unsigned long sgulRxDone = 1;

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context) {       
    switch(p_event->type) {
        case NRF_DRV_TWI_EVT_DONE:
            if(p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TX) {
                sgulTxDone = 1;
            } else {
                sgulRxDone = 1;
            }
            break;
        default:
            break;        
    }   
}

void LL_I2C__Init(void) { // void twi_init (void)
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t config = {
       .scl                = LL_PIN__I2C_SCL,//ARDUINO_SCL_PIN,
       .sda                = LL_PIN__I2C_SDA,//ARDUINO_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };
    
    err_code = nrf_drv_twi_init(&m_twi, &config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&m_twi);
}

void LL_I2C__UnInit(void) {
    nrf_drv_twi_uninit(&m_twi);
}

void LL_I2C__Tx(unsigned char device_address, unsigned char const *p_data, unsigned char length, bool no_stop) {
    sgulTxDone = 0;
    nrf_drv_twi_tx(&m_twi, device_address, p_data, length, no_stop);  
}
static unsigned char sguc_data_buf_of_tx_byte[2];
void LL_I2C__TxByte(unsigned char device_address, unsigned char reg_addr, unsigned char data) {
    sguc_data_buf_of_tx_byte[0] = reg_addr; sguc_data_buf_of_tx_byte[1] = data;     
    sgulTxDone = 0;
    nrf_drv_twi_tx(&m_twi, device_address, (unsigned char const *)sguc_data_buf_of_tx_byte, 2, false);  
}
void LL_I2C__TxChunk(unsigned char device_address, unsigned char *p_data, unsigned char length) {
    sgulTxDone = 0;
    nrf_drv_twi_tx(&m_twi, device_address, (unsigned char const *)p_data, length, false);  
}
unsigned long LL_I2C__TxDone(void) {
    return sgulTxDone;
}

void LL_I2C__Rx(unsigned char device_address, unsigned char *p_data, unsigned char length) {
    sgulRxDone = 0;
    nrf_drv_twi_rx(&m_twi, device_address, p_data, length);
}
unsigned long LL_I2C__RxDone(void) {
    return sgulRxDone;
}


