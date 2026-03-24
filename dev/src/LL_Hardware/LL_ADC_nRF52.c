/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

// Usage:
//      (None)
//
// Remarks:
//      Refer to: SDK11 \examples\peripheral\saadc\pca10040\arm4
//      Refer to: https://blog.csdn.net/polaris_zgx/article/details/80405334, a Chinese blog much better than official sample!
//      Pls set "SAADC_ENABLED" to be 1!


#include <stddef.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"//#include "nrf_adc.h"

#include "LL_Hardware_cfg.h"
#include "LL_ADC.h"



// 保存ADC采样值
#define SAMPLES_IN_BUFFER 3 // copied fromsample
static nrf_saadc_value_t m_buffer_pool[2][SAMPLES_IN_BUFFER]; // copied fromsample
volatile unsigned long LL_ADC_Sample    = LL_ADC_SAMPLE_VALUE_NONE;
volatile signed long LL_ADC_Sample_1    = LL_ADC_SAMPLE_VALUE_NONE;
volatile signed long LL_ADC_Sample_2    = LL_ADC_SAMPLE_VALUE_NONE;
volatile unsigned long LL_ADC_SampleSeq = 0;

//#ifndef NRF_APP_PRIORITY_HIGH
//#define NRF_APP_PRIORITY_HIGH 1
//#endif




// Copied from "saadc_callback()" of simple.
void saadc_callback(nrf_drv_saadc_evt_t const * p_event) {
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER); APP_ERROR_CHECK(err_code);

//        int i;
//        printf("ADC event number: %d\r\n",(int)m_adc_evt_counter);
//        for (i = 0; i < SAMPLES_IN_BUFFER; i++) {
//            printf("%d\r\n", p_event->data.done.p_buffer[i]);
//        }
//        m_adc_evt_counter++;
        LL_ADC_Sample_1 = p_event->data.done.p_buffer[0]; //charging
        if(LL_ADC_Sample_1 < 0){LL_ADC_Sample_1 = 0;}
        
        LL_ADC_Sample_2 = p_event->data.done.p_buffer[1]; //charging power
        if(LL_ADC_Sample_2 < 0){LL_ADC_Sample_2 = 0;}
        
        LL_ADC_Sample = p_event->data.done.p_buffer[2]; //battery
        LL_ADC_SampleSeq++;
    }
}

// Copied from "saadc_init()" of simple.
void LL_ADC_Init(void)
{
    ret_code_t err_code;
	
    err_code = nrf_drv_saadc_init(NULL, saadc_callback); APP_ERROR_CHECK(err_code); // The default "resolution" is 10-bit already, same as Lumos.
    
    //charging:AIN4
    nrf_saadc_channel_config_t channel_config_charging = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(LL_ADC_CH_NAME__CHARGING);    
    channel_config_charging.gain      = NRF_SAADC_GAIN1_6;            // 1/6 input
    channel_config_charging.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6V
    err_code = nrf_drv_saadc_channel_init(LL_ADC_CH_NUM__CHARGING, &channel_config_charging);APP_ERROR_CHECK(err_code);
    
    //charging standby:AIN5
    nrf_saadc_channel_config_t channel_config_charging_standby = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(LL_ADC_CH_NAME__CHARGING_STANDBY);
    channel_config_charging_standby.gain      = NRF_SAADC_GAIN1_6;            // 1/6 input
    channel_config_charging_standby.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6V
    err_code = nrf_drv_saadc_channel_init(LL_ADC_CH_NUM__CHARGING_STANDBY, &channel_config_charging_standby);APP_ERROR_CHECK(err_code);
    
    // Lumos: hardware input is 1/3 (or 1/4 ?) VDD, firmware setting is "FULL_SCALE" and "1.2V ref".
    // Selina has the same hardware input, so let's scale it by 1/2, since the ref of nRF52 becomes 0.6V which is 1/2 of nRF51.
    nrf_saadc_channel_config_t channel_config_battery = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(LL_ADC_CH_NAME__BATTERY);
    channel_config_battery.gain      = NRF_SAADC_GAIN1_2;            // 1/2 input
    channel_config_battery.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6V
    err_code = nrf_drv_saadc_channel_init(LL_ADC_CH_NUM__BATTERY, &channel_config_battery); APP_ERROR_CHECK(err_code);
    
    // Though I still don't know WTF dual bank buffer!
    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0],SAMPLES_IN_BUFFER); APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1],SAMPLES_IN_BUFFER); APP_ERROR_CHECK(err_code);
}

// No sample!
void LL_ADC_StartSingleConversion(void)//(unsigned long ulCH)
{
//    nrf_adc_config_input_t ch;
//    switch (ulCH) {
//        case 0: ch = NRF_ADC_CONFIG_INPUT_0; break;
//        case 1: ch = NRF_ADC_CONFIG_INPUT_1; break;
//        case 2: ch = NRF_ADC_CONFIG_INPUT_2; break;
//        case 3: ch = NRF_ADC_CONFIG_INPUT_3; break;
//        case 4: ch = NRF_ADC_CONFIG_INPUT_4; break;
//        case 5: ch = NRF_ADC_CONFIG_INPUT_5; break;
//        case 6: ch = NRF_ADC_CONFIG_INPUT_6; break;
//        case 7: ch = NRF_ADC_CONFIG_INPUT_7; break;
//        default : return; // Error: do not start any conversion!
//    }
//    nrf_adc_input_select(ch);
//    nrf_adc_start();
    
//    // fake fake fake
//    LL_ADC_Sample = 836; LL_ADC_SampleSeq++;

    nrf_drv_saadc_sample(); // right?
}
