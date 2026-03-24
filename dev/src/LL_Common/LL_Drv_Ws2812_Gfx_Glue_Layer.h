
#ifndef LL_DRV_WS2812_GFX_GLUE_LAYER_H__
#define LL_DRV_WS2812_GFX_GLUE_LAYER_H__

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_gpiote.h"


// Function for initializing the LCD controller.

ret_code_t LL_Drv_Ws2812_Reset(void);

void LL_Drv_Ws2812_Set_Solid_Mode(void);
void LL_Drv_Ws2812_Set_Slow_Flash_Mode(uint8_t timecnt);
void LL_Drv_Ws2812_TurnLeft(uint8_t timecnt);
void LL_Drv_Ws2812_TurnRight(uint8_t timecnt);

void LL_Drv_Ws2812_Mode(uint32_t LedType);
void LL_Drv_Ws2812_Mode0(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode1(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode2(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode2(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode3(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode4(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode5(uint8_t timecnt);
ret_code_t LL_Drv_Ws2812_Mode6(uint8_t timecnt);
void LL_Drv_Ws2812_Mode7(uint8_t timecnt);
void LL_WS2812_SetFlashPattern(uint8_t glWs2812Mode);
#endif // WS2812_GFX_GLUE_LAYER_H__

