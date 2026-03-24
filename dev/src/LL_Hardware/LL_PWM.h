/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_PWM_H
#define _LL_PWM_H
#include "stdio.h"
#include "stdint.h"
#include "LL_LED_Panel_WS2812.h"
#include "LL_Hardware_cfg.h"

// NOTE: 
// The hardware PWM is LL_HW_PWM_PERIOD us period,            0~100 duty,  2 channel.
// The software PWM is a period of calling of LL_PWM_perDuty, 0~100 duty, 16 channel.

typedef struct
{
    unsigned long N;        // user define number, should be [0, LL_PWM_CH_NUM).
    unsigned long HW;       // hardware implement, 1: nRF51(Timer+GPIOTE), 2: nRF52 Real PWM
    unsigned long HW_CH;    // hardware channel, 0/1 when HW is 1, 0/1/2/3 when HW is 2
    unsigned long port;     // GPIO port
    unsigned long pin;      // GPIO pin
    unsigned long normal;   // 0/1 when OFF.
    unsigned long pull;     // see LL_GPIO.h LL_GPIO_PULL_xxx. meaningless when HW = 1.
}T_LL_PWM_CFG;
extern T_LL_PWM_CFG gtPWM[LL_PWM_CH_NUM];

// Interface of PWM module:

#define LL_PWM_PERIOD_us(freq) (1000*1000/(freq))  // 1000 ms/s * 1000 us/ms
void LL_PWM_Init(unsigned long freq);//void LL_PWM_Init(void);
//void LL_PWM_HwStop(void); // nRF51 need this before a new config!
void LL_PWM_Sleep(void); // nRF52 PWM needs some operation
void LL_PWM_perDuty(void);

/**
 * @brief This function must be called to draw the actual buffer onto the LED matrix
 */
uint32_t LL_Drv_Ws2812_Display(void);
// Interface of PWM channel:
unsigned long LL_PWM_isON(    unsigned long ch                      );
void          LL_PWM_ON(      unsigned long ch                      );
unsigned long LL_PWM_isOFF(   unsigned long ch                      );
void          LL_PWM_OFF(     unsigned long ch                      );
void          LL_PWM_Toggle(  unsigned long ch                      );
void          LL_PWM_DutySet( unsigned long ch, unsigned long duty  );
unsigned long LL_PWM_DutyGet( unsigned long ch                      );
//
void LL_PWM_DutySet_noSafeDuty(unsigned long ch, unsigned long duty); // for testing

//ws2812 
void LL_WS2812_BufferInit(void);

uint32_t LL_WS2812_Init(void);

/**
 * @brief Draws a single pixel in the buffer. 
 *          drv_ws2812_display() must be called to update the LED matrix
 */
uint32_t LL_Drv_Ws2812_Pixel_Draw( uint16_t LampBeadOnX, uint16_t LampBeadOnY, uint32_t LedColor);

/**
 * @brief Draws a rectangle in the buffer. 
 *          drv_ws2812_display() must be called to update the LED matrix
 *          BE AWARE OF BUG IN GFX library: https://devzone.nordicsemi.com/f/nordic-q-a/37284/bug-gfx-line-drawing-mishandles-start-end-values
 */
uint32_t LL_Drv_Ws2812_Rectangle_Draw(uint16_t LampBeadOnX, uint16_t LampBeadOnY, uint16_t LampBeadNumOn, uint16_t LightBarNum, uint32_t LedColor);

void LL_Drv_Ws2812_Front_Draw(uint16_t NumOfFrameFront);
void LL_Drv_Ws2812_Rear_Draw(uint16_t NumOfFrameRear);
void LL_Convert_Rgb_To_Pwm_Sequence(void);
void LL_Convert_Rgb_To_Pwm_Sequence_Channel0(void);
void LL_Convert_Rgb_To_Pwm_Sequence_Channel1(void);

#endif
