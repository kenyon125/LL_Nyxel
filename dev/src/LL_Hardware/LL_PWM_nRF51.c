/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "app_pwm.h"
#include "nrf_drv_pwm.h"
#include "nrfx_pwm.h"

#include "LL_Hardware_cfg.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_PWM.h"
#include "LL_LED_Helmet.h"
#include "LL_Para.h"

APP_PWM_INSTANCE(PWM1, 1); // Create the instance "PWM1" using TIMER1.


#if 1 // nRF52 Real PWM, copied from "\nRF5_SDK_14.0.0_3bcc1f7\examples\peripheral\pwm_driver\main.c"
#include "app_util_platform.h" // APP_IRQ_PRIORITY_LOWEST
#include "nrf_drv_pwm.h" 

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);
//static nrf_drv_pwm_t m_pwm1 = NRF_DRV_PWM_INSTANCE(1);
//static nrf_drv_pwm_t m_pwm2 = NRF_DRV_PWM_INSTANCE(2);

// This is for tracking PWM instances being used, so we can unintialize only the relevant ones when switching from one demo to another.
#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;

static uint16_t const              m_demo1_top  = 20;//1000 // can hear noise when >= 2000,ws2812 should set 20
// WS2812 protocol requires a 800 kHz PWM frequency. PWM Top value = 20 and Base Clock = 16 MHz achieves this
//static uint16_t const              m_demo1_step = 200;
//static uint8_t                     m_demo1_phase;
static nrf_pwm_values_individual_t m_demo1_seq_values;
static rgb_color_t led_matrix_buffer[LL_LED_MATRIX_WIDTH][LL_LED_MATRIX_HEIGHT];
static nrf_pwm_values_individual_t pwm_duty_cycle_values[LED_MATRIX_TOTAL_BIT_WIDTH];

static nrf_pwm_sequence_t pwm_sequence =
{
    .values.p_individual = pwm_duty_cycle_values,
    .length          = (sizeof(pwm_duty_cycle_values) / sizeof(uint16_t)),
    .repeats         = 0,
    .end_delay       = 0
};
// This is for tracking PWM instances being used, so we can unintialize only the relevant ones when switching from one demo to another.
#define USED_PWM(idx) (1UL << idx)

volatile bool pwm_sequencue_finished = true;

void pwm_handler(nrf_drv_pwm_evt_type_t event_type)
{
    switch(event_type)
    {
	case NRF_DRV_PWM_EVT_FINISHED:
	    pwm_sequencue_finished = true;
	    break;
	default:
	    break;
    }
}

nrf_drv_pwm_config_t config0 = {
    .output_pins = {
        NRF_DRV_PWM_PIN_NOT_USED, // channel 0
        NRF_DRV_PWM_PIN_NOT_USED, // channel 1
        NRF_DRV_PWM_PIN_NOT_USED, // channel 2
        NRF_DRV_PWM_PIN_NOT_USED  // channel 3
    },
    .irq_priority = APP_IRQ_PRIORITY_LOWEST,
    .base_clock   = NRF_PWM_CLK_16MHz,
    .count_mode   = NRF_PWM_MODE_UP,
    .top_value    = m_demo1_top,
    .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
    .step_mode    = NRF_PWM_STEP_AUTO
};
#endif
// Xiong create this for better coding
uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values; 
#define LL_Duty_of_nRF52_PWM(ch, duty)  ( ((1 != gtPWM[ch].normal) ? (100-duty) : (duty)) * (m_demo1_top/100) )

T_LL_PWM_CFG gtPWM[LL_PWM_CH_NUM] = LL_PWM_CFG;

static unsigned long sgulStateCH;    // 0:OFF   1:ON
unsigned long LL_PWM_isON(  unsigned long ch) { if( 1 == ((sgulStateCH&(0x1<<ch))>>ch) ) { return 1; } else { return 0; } }
unsigned long LL_PWM_isOFF( unsigned long ch) { if( 0 == ((sgulStateCH&(0x1<<ch))>>ch) ) { return 1; } else { return 0; } }

static unsigned long sgulStateDuty;  // 0:idle  1:active
static unsigned long sgaulDuty[LL_PWM_CH_NUM];
static unsigned long sgulDutyCnt;

void LL_PWM_Init(unsigned long freq)//void LL_PWM_Init(void)
{
    app_pwm_config_t pwm1_cfg; 
    pwm1_cfg.pins[0] = APP_PWM_NOPIN;
    pwm1_cfg.pins[1] = APP_PWM_NOPIN;
    pwm1_cfg.num_of_channels = 0;
    pwm1_cfg.period_us = freq;//LL_HW_PWM_PERIOD;
    
    unsigned long ulHwChNum       = 0; // counter for the nRF51 PWM(Timer+GPIOTE)
    unsigned long ulHwChNum_nRF52 = 0; // counter for the nRF52 PWM
    for(int ch = 0; ch < LL_PWM_CH_NUM; ch++) {
        if(0 == gtPWM[ch].HW) { // software
            LL_GPIO_OutputCfg(gtPWM[ch].port, gtPWM[ch].pin, gtPWM[ch].pull, gtPWM[ch].normal);
        } else if(1 == gtPWM[ch].HW) { // nRF51 PWM(Timer+GPIOTE)
            ulHwChNum++;
            // pins:
            pwm1_cfg.pins[pwm1_cfg.num_of_channels] = gtPWM[ch].pin;
            // pin_polarity:
            if(0 == gtPWM[ch].normal) { pwm1_cfg.pin_polarity[pwm1_cfg.num_of_channels] = APP_PWM_POLARITY_ACTIVE_HIGH; }
            else                      { pwm1_cfg.pin_polarity[pwm1_cfg.num_of_channels] = APP_PWM_POLARITY_ACTIVE_LOW;  }
            // num_of_channels
            if(1 > pwm1_cfg.num_of_channels) { pwm1_cfg.num_of_channels++; }
        } else if(2 == gtPWM[ch].HW) { // nRF52 Real PWM
            if(4 > ulHwChNum_nRF52) { ulHwChNum_nRF52++;
                config0.output_pins[gtPWM[ch].HW_CH] = gtPWM[ch].pin; if(1 == gtPWM[ch].normal) { config0.output_pins[gtPWM[ch].HW_CH] |= NRF_DRV_PWM_PIN_INVERTED; }
                // set duty to 0 as default
                //p_channels[gtPWM[ch].HW_CH] = LL_Duty_of_nRF52_PWM(ch, 0);
            }
        } else { // unknown
            continue; // next pin
        }
        sgaulDuty[ch] = 0;  
    }
    sgulStateCH   = 0x00000000;
    sgulStateDuty = 0x00000000;
    sgulDutyCnt   = 0x00000000;
    
    // Hardware PWM needs to do something more:
    if(       0 == ulHwChNum) {
        // no hardware PWM, it's ok.
    } else if(1 == ulHwChNum) {
        app_pwm_init(&PWM1, &pwm1_cfg, 0);
        app_pwm_channel_duty_set(&PWM1, 0, 0);
//      app_pwm_channel_duty_set(&PWM1, 1, 0);
        app_pwm_enable(&PWM1);
    } else if(2 == ulHwChNum) {
        app_pwm_init(&PWM1, &pwm1_cfg, 0);
        app_pwm_channel_duty_set(&PWM1, 0, 0);
        app_pwm_channel_duty_set(&PWM1, 1, 0);
        app_pwm_enable(&PWM1);
    } else {
        // error!
    }

    // nRF52 PWM needs to do something more:
    if(0 == ulHwChNum_nRF52) { // no hardware PWM
        // nothing
    } else { // some PWM
        APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, pwm_handler)); m_used |= USED_PWM(0);
        LL_WS2812_BufferInit();
    }
    
}

void LL_PWM_Sleep(void) {
    app_pwm_uninit(&PWM1);
    
    // copied from bsp_evt_handler of: \nRF5SDK1400\nRF5_SDK_14.0.0_3bcc1f7\examples\peripheral\pwm_driver\main.c
    if (m_used & USED_PWM(0))
    {
        nrf_drv_pwm_uninit(&m_pwm0);
    }
    
//    if (m_used & USED_PWM(2))
//    {
//        nrf_drv_pwm_uninit(&m_pwm2);
//    }
    m_used = 0;
}

static unsigned long get_a_safe_duty(unsigned long ch, unsigned long duty) 
{
    #ifdef _LL_NO_SAFE_DUTY
        return duty;
    #else
    { // there is a safe duty
        //
        // no need for buzzer
        E_LL_LED eLED = (E_LL_LED)(ch); if(E_LL_PWM_BUZZER == eLED) { return duty; }
        //
        // get the flashing bar number
        unsigned long count_of_1_in_flashing = 0; {
            unsigned long flashing = LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, eLED);
            for(int i = 0; i < LL_LED_CUSTOMER_FLASHING_TIMESLOT_NUM; i++) {
                if(1 == ((flashing>>i)&0x1)) { count_of_1_in_flashing++; }
            } if(0 == count_of_1_in_flashing) { count_of_1_in_flashing = 1; } // avoid divide 0
        }
        //
        // get the cap acc to the flashing bar number
        unsigned long max_duty_of_this_flashing; {
            unsigned long max_duty_when_solid = 60; // 60 is acc to JIRA https://lumoshelmet.atlassian.net/browse/LF-24?atlOrigin=eyJpIjoiMTRlMTAwZjBlM2ViNDljZjlkOWEyMDY1ZDhkMjkyNDIiLCJwIjoiaiJ9
            max_duty_of_this_flashing = max_duty_when_solid * LL_LED_CUSTOMER_FLASHING_TIMESLOT_NUM / count_of_1_in_flashing; if(max_duty_of_this_flashing > 100) { max_duty_of_this_flashing = 100; }
        }
                 
        // return the duty itself or the cap (if exceed)
        return ((duty <= max_duty_of_this_flashing) ? (duty) : (max_duty_of_this_flashing)); // a simple "cap"
    }
    #endif
}

void LL_PWM_ON(  unsigned long ch)
{
    unsigned long safe_duty = get_a_safe_duty(ch, sgaulDuty[ch]);
    if( LL_PWM_isOFF(ch) ) { 
        if(     1 == gtPWM[ch].HW) { while(NRF_ERROR_BUSY == app_pwm_channel_duty_set(&PWM1, gtPWM[ch].HW_CH, safe_duty));    }
        else if(2 == gtPWM[ch].HW) { p_channels[gtPWM[ch].HW_CH] = LL_Duty_of_nRF52_PWM(ch, safe_duty); }
        else                       { LL_GPIO_OutputWrite(gtPWM[ch].port, gtPWM[ch].pin, (gtPWM[ch].normal + 1)&0x1);
                                     sgulStateDuty &= ~(0x1<<ch);                                                    }
        sgulStateCH |= 0x1<<ch;        
    }
}
void LL_PWM_OFF( unsigned long ch)
{
    if( LL_PWM_isON(ch) ) { 
        if(     1 == gtPWM[ch].HW) { while(NRF_ERROR_BUSY == app_pwm_channel_duty_set(&PWM1, gtPWM[ch].HW_CH,   0));    }
        else if(2 == gtPWM[ch].HW) { p_channels[gtPWM[ch].HW_CH] = LL_Duty_of_nRF52_PWM(ch, 0); }
        else                       { LL_GPIO_OutputWrite(gtPWM[ch].port, gtPWM[ch].pin, gtPWM[ch].normal); }
        sgulStateCH &= ~(0x1<<ch);        
    }
}
void LL_PWM_Toggle( unsigned long ch)
{
    if( LL_PWM_isON(ch) ) { LL_PWM_OFF(ch); } else { LL_PWM_ON(ch); }
}

void LL_PWM_DutySet(unsigned long ch, unsigned long duty)
{
    unsigned long safe_duty = get_a_safe_duty(ch, duty);
    if( (1 == gtPWM[ch].HW) && (LL_PWM_isON(ch)) ) { while(NRF_ERROR_BUSY == app_pwm_channel_duty_set(&PWM1, gtPWM[ch].HW_CH, safe_duty)); }
    if( (2 == gtPWM[ch].HW) && (LL_PWM_isON(ch)) ) { p_channels[gtPWM[ch].HW_CH] = LL_Duty_of_nRF52_PWM(ch, safe_duty); }
    sgaulDuty[ch] = duty;
}
void LL_PWM_DutySet_noSafeDuty(unsigned long ch, unsigned long duty)
{
    unsigned long safe_duty = duty;//get_a_safe_duty(ch, duty);
    if( (1 == gtPWM[ch].HW) && (LL_PWM_isON(ch)) ) { while(NRF_ERROR_BUSY == app_pwm_channel_duty_set(&PWM1, gtPWM[ch].HW_CH, safe_duty)); }
    if( (2 == gtPWM[ch].HW) && (LL_PWM_isON(ch)) ) { p_channels[gtPWM[ch].HW_CH] = LL_Duty_of_nRF52_PWM(ch, safe_duty); }
    sgaulDuty[ch] = duty;
}
unsigned long LL_PWM_DutyGet(unsigned long ch)
{
    return sgaulDuty[ch];
}

void LL_PWM_perDuty(void)
{    
    sgulDutyCnt++; if(100 <= sgulDutyCnt) { sgulDutyCnt = 0; sgulStateDuty = 0x00000000; }
    for(int ch = 0; ch < LL_PWM_CH_NUM; ch++) {
        if( LL_PWM_isON(ch) ) {
            if(0 == gtPWM[ch].HW) {
                if(sgaulDuty[ch] > sgulDutyCnt) { if( 0 == ((sgulStateDuty&(0x1<<ch))>>ch) ) { LL_GPIO_OutputWrite(gtPWM[ch].port, gtPWM[ch].pin, (gtPWM[ch].normal + 1)&0x1 ); sgulStateDuty |=  (0x1<<ch); } }
                else                            { if( 1 == ((sgulStateDuty&(0x1<<ch))>>ch) ) { LL_GPIO_OutputWrite(gtPWM[ch].port, gtPWM[ch].pin,  gtPWM[ch].normal          ); sgulStateDuty &= ~(0x1<<ch); } }
            } else {
                // nothing for hardware PWM.
            }
        } else {
            // nothing for a OFF channel.
        }
    }
}

uint32_t LL_Drv_Ws2812_Display(void)
{
    if(!pwm_sequencue_finished) 
    {
        return NRF_ERROR_BUSY;
    }
    //convert_rgb_to_pwm_sequence();
    pwm_sequencue_finished = false;
    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &pwm_sequence, 1, NRF_DRV_PWM_FLAG_STOP);
    
    return NRF_SUCCESS;
}

void LL_Convert_Rgb_To_Pwm_Sequence(void)
{
    uint8_t * ptr = (uint8_t *)led_matrix_buffer;
    uint32_t i = 0;
    for(int led = 0; led < LED_MATRIX_TOTAL_BYTE_WIDTH; led++)
    {
        for(int bit = 7; bit >= 0; bit--)
        {
            uint8_t code = (*ptr >> bit) & 0x01;
            uint16_t pwm_value = 0;
            if(code == 1)
            {
                pwm_value = WS2812_T1H;
            }
            else
            {
                pwm_value = WS2812_T0H;
            }
            pwm_duty_cycle_values[i].channel_1 = pwm_value;
            pwm_duty_cycle_values[i].channel_0 = pwm_value;
						
            i++;
        }
        ptr++;
    }
}

void LL_Convert_Rgb_To_Pwm_Sequence_Channel0(void)
{
    uint8_t * ptr = (uint8_t *)led_matrix_buffer;
    uint32_t i = 0;
    for(int led = 0; led < LED_MATRIX_TOTAL_BYTE_WIDTH; led++)
    {
        for(int bit = 7; bit >= 0; bit--)
        {
            uint8_t code = (*ptr >> bit) & 0x01;
            uint16_t pwm_value = 0;
            if(code == 1)
            {
                pwm_value = WS2812_T1H;
            }
            else
            {
                pwm_value = WS2812_T0H;
            }
            pwm_duty_cycle_values[i++].channel_0 = pwm_value;
        }
        ptr++;
    }
}

void LL_Convert_Rgb_To_Pwm_Sequence_Channel1(void)
{
    uint8_t * ptr = (uint8_t *)led_matrix_buffer;
    uint32_t i = 0;
    for(int led = 0; led < LED_MATRIX_TOTAL_BYTE_WIDTH; led++)
    {
        for(int bit = 7; bit >= 0; bit--)
        {
            uint8_t code = (*ptr >> bit) & 0x01;
            uint16_t pwm_value = 0;
            if(code == 1)
            {
                pwm_value = WS2812_T1H;
            }
            else
            {
                pwm_value = WS2812_T0H;
            }
            pwm_duty_cycle_values[i++].channel_1 = pwm_value;
        }
        ptr++;
    }
}

void LL_WS2812_BufferInit(void)
{   
    memset(led_matrix_buffer, 0x00, sizeof(led_matrix_buffer));
}

void LL_Drv_Ws2812_Front_Draw(uint16_t NumOfFrameFront)
{
    uint8_t LedCnt;
    for(LedCnt=0; LedCnt<LL_LED_MATRIX_WIDTH; LedCnt++){
        led_matrix_buffer[LedCnt][0].r = (gtPanelPara.animationFramesFront_flash[NumOfFrameFront][LedCnt] & 0x00FF0000) >> 16;
        led_matrix_buffer[LedCnt][0].g = (gtPanelPara.animationFramesFront_flash[NumOfFrameFront][LedCnt] & 0x0000FF00) >> 8;
        led_matrix_buffer[LedCnt][0].b = (gtPanelPara.animationFramesFront_flash[NumOfFrameFront][LedCnt] & 0x000000FF);
    }
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
    
    //reset
    LL_Drv_Ws2812_Reset();         
}

void LL_Drv_Ws2812_Rear_Draw(uint16_t NumOfFrameRear)
{
    uint8_t LedCnt;
    for(LedCnt=0; LedCnt<LL_LED_MATRIX_WIDTH; LedCnt++){
        led_matrix_buffer[LedCnt][0].r = (gtPanelPara.animationFramesRear_flash[NumOfFrameRear][LedCnt] & 0x00FF0000) >> 16;
        led_matrix_buffer[LedCnt][0].g = (gtPanelPara.animationFramesRear_flash[NumOfFrameRear][LedCnt] & 0x0000FF00) >> 8;
        led_matrix_buffer[LedCnt][0].b = (gtPanelPara.animationFramesRear_flash[NumOfFrameRear][LedCnt] & 0x000000FF);
    }
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    
    //reset
    LL_Drv_Ws2812_Reset();   
}


uint32_t LL_Drv_Ws2812_Pixel_Draw(uint16_t LampBeadOnX, uint16_t LampBeadOnY, uint32_t LedColor)
{
    uint32_t err_code = NRF_SUCCESS;
    if(LampBeadOnX > LL_LED_MATRIX_WIDTH - 1)
    {
       err_code = NRF_ERROR_INVALID_PARAM;
    }
    if(LampBeadOnY > LL_LED_MATRIX_HEIGHT - 1)
    {
			err_code = NRF_ERROR_INVALID_PARAM;
    }
 
    led_matrix_buffer[LampBeadOnX][LampBeadOnY].r = (LedColor & 0x00FF0000) >> 16;
    led_matrix_buffer[LampBeadOnX][LampBeadOnY].g = (LedColor & 0x0000FF00) >> 8;
    led_matrix_buffer[LampBeadOnX][LampBeadOnY].b = (LedColor & 0x000000FF);
    
    return err_code;
}


//uint32_t LL_Drv_Ws2812_Rectangle_Draw(uint16_t LampBeadOnX, uint16_t LampBeadOnY, uint16_t LampBeadNumOn, uint16_t LightBarNum, uint32_t LedColor)
//{
//    uint32_t err_code;
//    for(int i = LampBeadOnY; i < (LampBeadOnY + LightBarNum); i++)
//    {
//        for(int j = LampBeadOnX; j < (LampBeadOnX + LampBeadNumOn); j++)
//        {
//            err_code = LL_Drv_Ws2812_Pixel_Draw(j, i, LedColor);
//            if(err_code) return err_code;
//        }
//    }
//		
//    return NRF_SUCCESS;
//}

