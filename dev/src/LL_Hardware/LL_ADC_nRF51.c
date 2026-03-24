/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/


#include <stddef.h>
#include "nrf.h"
#include "nrf51.h"
#include "nrf_adc.h"

#include "LL_ADC.h"



// 保存ADC采样值
volatile unsigned long LL_ADC_Sample    = LL_ADC_SAMPLE_VALUE_NONE;
volatile unsigned long LL_ADC_SampleSeq = 0;

#ifndef NRF_APP_PRIORITY_HIGH
#define NRF_APP_PRIORITY_HIGH 1
#endif

/*******************************************************************************
 @brief ADC interrupt handler.
 @details copy from "\examples\peripheral\adc_simple".
 @param[in] void
 @param[out] void
 @param[in,out] void
 @retval void
*******************************************************************************/
void ADC_IRQHandler(void)
{
    nrf_adc_conversion_event_clean();

    LL_ADC_Sample = nrf_adc_result_get(); LL_ADC_SampleSeq++;
}


void LL_ADC_Init(void)
{
    nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG_DEFAULT;

    // Initialize and configure ADC
    nrf_adc_config.resolution = NRF_ADC_CONFIG_RES_10BIT;
    nrf_adc_config.scaling    = NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE;
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
    nrf_adc_input_select(NRF_ADC_CONFIG_INPUT_DISABLED);
    nrf_adc_int_enable(ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos);
    NVIC_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_HIGH);
    NVIC_EnableIRQ(ADC_IRQn);
}

void LL_ADC_StartSingleConversion(unsigned long ulCH)
{
    nrf_adc_config_input_t ch;
    switch (ulCH) {
        case 0: ch = NRF_ADC_CONFIG_INPUT_0; break;
        case 1: ch = NRF_ADC_CONFIG_INPUT_1; break;
        case 2: ch = NRF_ADC_CONFIG_INPUT_2; break;
        case 3: ch = NRF_ADC_CONFIG_INPUT_3; break;
        case 4: ch = NRF_ADC_CONFIG_INPUT_4; break;
        case 5: ch = NRF_ADC_CONFIG_INPUT_5; break;
        case 6: ch = NRF_ADC_CONFIG_INPUT_6; break;
        case 7: ch = NRF_ADC_CONFIG_INPUT_7; break;
        default : return; // Error: do not start any conversion!
    }
    nrf_adc_input_select(ch);
    nrf_adc_start();
}
