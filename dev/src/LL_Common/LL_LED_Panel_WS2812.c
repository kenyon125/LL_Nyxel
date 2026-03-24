
#include <stdbool.h>
#include <stdint.h>
#include "nrf_assert.h"
#include "nrf_gpio.h"
#include "sdk_errors.h"
#include "LL_PWM.h"
#include "LL_Para.h"
#include "LL_SysMode.h"
#include "LL_LED_Panel_WS2812.h"
#include "LL_LED_Panel_Data_SpecialSignal.h"
#include "LL_LED_Panel_Data_CommonFlash.h"
#include "LL_Battery.h"
#include "LL_Clock.h"
#include "LL_Timer.h"

static const unsigned long animation_color_corresponding_code[] = {  WS2812_COLOR_BLACK, WS2812_COLOR_RED, WS2812_COLOR_WHITE, WS2812_COLOR_YELLOW, WS2812_COLOR_ORANGE, WS2812_COLOR_GREEN, 
                                                        WS2812_COLOR_BLUE, WS2812_COLOR_PURPLE, WS2812_COLOR_WHITE_100, WS2812_COLOR_RED_100, WS2812_COLOR_TURNSIGNAL, 
                                                        WS2812_COLOR_CHARGING_RED, WS2812_COLOR_CHARGING_ORANGE, WS2812_COLOR_CHARGING_GREEN};
static const unsigned long animation_color_poweron_corresponding_code[] = {WS2812_COLOR_BLACK, 0x00000F, 0x000014, 0x00002D, 0x00003E, 0x00004B, 0x00005A, 0x000069, 0x000078, 0x000087, 0x000096, 0x0000A5, 0x0000B4, 0x0000C3, 0x0000D2, 0x0000E1, 0x0000FF};
//void LL_Drv_Ws2812_SyncWithClock(unsigned long time_snapshot, E_LL_LED led)
void LL_Drv_Ws2812_SyncWithClock(E_LL_LED led)
{
    unsigned long total_bar_num = 12;         
    unsigned long total_timeslot_num = 48;    //1/48


    unsigned long setting_of_all_bar = (led == E_LL_LED_FRONT)
                                           ? LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, E_LL_LED_FRONT)
                                           : LL_Para__get_flashing_pattern(gtPara.eModeOfWarningLight, E_LL_LED_REAR);

    for (unsigned long timeslot = 0; timeslot < total_timeslot_num; timeslot++)
    {
        unsigned long index_of_now_bar = (timeslot * total_bar_num) / total_timeslot_num;
        unsigned long index_in_bar_of_now_timeslot = timeslot % (total_timeslot_num / total_bar_num);

        unsigned long setting_of_now_bar = GET_BAR_STATE(setting_of_all_bar, index_of_now_bar);

        unsigned long need_ON = 0;
        switch (setting_of_now_bar)
        {
        case 0: // OFF
            need_ON = 0;
            break;
        case 1:
        case 2: // half blink (1/48)
            if (0 == index_in_bar_of_now_timeslot)
            {
                need_ON = 1;
            }
            break;
        case 3: // ON
            need_ON = 1;
            break;
        default:
            need_ON = 0;
            break;
        }
        for (unsigned long led_index = 0; led_index < LL_LED_MATRIX_WIDTH; led_index++)
        {
            if(led == E_LL_LED_FRONT){
                gtPanelPara.animationFramesFront_flash[timeslot][led_index] = (need_ON) ? gtPara.brightness_individual[gtPara.eModeOfWarningLight][E_LL_LED_FRONT] : 0; // 
            }else{
                gtPanelPara.animationFramesRear_flash[timeslot][led_index] = (need_ON) ? gtPara.brightness_individual[gtPara.eModeOfWarningLight][E_LL_LED_REAR] : 0; // 
            }
        }
    }
}


void LL_Convert_Frames(const T_FRAME__COLOR_CODE_FORMAT* inputFrames, int frameCount, T_FRAME__RGB_FORMAT* outputFrames) {
    unsigned char FrameCycle;
    if (inputFrames == NULL || frameCount == 0 || frameCount > 48 ) {
        return; 
    }
    
    for (int i = 0; i < frameCount; i++) {
        //outputFrames[i].stay_ms = inputFrames[i].stay_ms;

        memset(outputFrames[i].data, 0, sizeof(outputFrames[i].data)); 
        for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
            
            unsigned char color_code = inputFrames[i].color_code[j]; 
            
            // check code if overflow
            if (color_code < COLOR_CODE_NUM) {
                outputFrames[i].data[j] = animation_color_corresponding_code[color_code];
            }else{
                outputFrames[i].data[j] = 0;
            }
                
        }
    }
    
    //if need loop, such as 2/3/4/6/8/12/16/24 frames
    FrameCycle = LL_ANIMATION_FRAME_NUM/frameCount;
    if(FrameCycle > 1){ 
        for(int i = 1; i < FrameCycle; i++){
            for(int j = 0; j < frameCount; j++){
                for (int k = 0; k < LL_LED_MATRIX_WIDTH; k++) {
                    outputFrames[j+ frameCount*i].data[k] = outputFrames[j].data[k];
                }
            }
        }
    }
}

//power on
void LL_Convert_Frames_PowerOn(const T_FRAME__COLOR_CODE_FORMAT* inputFrames, int frameCount, T_FRAME__RGB_FORMAT* outputFrames, E_LL_LED led, unsigned int glBatLevelDisplayNum) {
    unsigned char colorCode;
    unsigned long BatLevelColor;
    if (inputFrames == NULL || frameCount == 0) {
        return; 
    }
    
    for (int i = 0; i < frameCount; i++) {
        //outputFrames[i].stay_ms = inputFrames[i].stay_ms;

        memset(outputFrames[i].data, 0, sizeof(outputFrames[i].data)); 
        for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
            colorCode = inputFrames[i].color_code[j]; 
            
            // check code if overflow
            if (colorCode < COLOR_CODE_NUM && colorCode > DEFAULT_BLACK) {
                outputFrames[i].data[j] = animation_color_corresponding_code[colorCode];
                BatLevelColor = animation_color_corresponding_code[colorCode];
            }else{
                outputFrames[i].data[j] = 0;
            }
                
        }
    }
    

    if(led == E_LL_LED_FRONT){
        for(int j = frameCount; j < LL_ANIMATION_FRAME_NUM; j++){
            for (int i = 0; i < glBatLevelDisplayNum; i++) {
                outputFrames[j].data[i] = BatLevelColor;
            }      
        }                
    }else{
        for(int j = frameCount; j < LL_ANIMATION_FRAME_NUM; j++){
            for (int i = LL_LED_MATRIX_WIDTH-1; i >= LL_LED_MATRIX_WIDTH - glBatLevelDisplayNum && i>=0; i--) {
                outputFrames[j].data[i] = BatLevelColor;
            }
        }
    }
}

//Charging
void LL_Convert_Frames_Charging(const T_FRAME__COLOR_CODE_FORMAT* inputFrames, int frameCount, T_FRAME__RGB_FORMAT* outputFrames) {
    if (inputFrames == NULL || frameCount == 0 || frameCount > 48 ) {
        return; 
    }
    
    for (int i = 0; i < frameCount; i++) {
        //outputFrames[i].stay_ms = inputFrames[i].stay_ms;

        memset(outputFrames[i].data, 0, sizeof(outputFrames[i].data)); 
        for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
            
            unsigned char color_code = inputFrames[i].color_code[j]; 
            
            // check code if overflow
            if (color_code < POWERON_BLUE_NUM) {
                outputFrames[i].data[j] = animation_color_poweron_corresponding_code[color_code];
            }else{
                outputFrames[i].data[j] = 0;
            }
        }
    } 
}

ret_code_t LL_Drv_Ws2812_Reset(void)
{
    //reset
	uint8_t i = 0;
	uint32_t err_code;
	//reset
	for(i = 0; i < LL_LED_MATRIX_WIDTH; i++)
	{
		err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_BLACK);
		if(err_code) return err_code;				
	}  
    return NRF_SUCCESS;
}

////mode: full beam FronWhite & RearRed
//void LL_Drv_Ws2812_Set_Solid_Mode(void)
//{
//	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LL_LED_MATRIX_WIDTH, LL_LED_MATRIX_HEIGHT, WS2812_COLOR_WHITE_20);
//	LL_Convert_Rgb_To_Pwm_Sequence_Channel0();  

//    //reset
//    LL_Drv_Ws2812_Reset();
//    
//	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LL_LED_MATRIX_WIDTH, LL_LED_MATRIX_HEIGHT, WS2812_COLOR_RED_20);
//	LL_Convert_Rgb_To_Pwm_Sequence_Channel1();    
//    
//    //reset
//    LL_Drv_Ws2812_Reset();    
//}

//brake
void LL_LED_Panel_BrakeAnimation(uint32_t AnimationColor, E_LL_LED led)
{
    if(led == E_LL_LED_FRONT){
        for(int i = 0; i < LL_ANIMATION_FRAME_NUM; i+=2){ 
            for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
                gtPanelPara.animationFramesFront_flash[i][j] = AnimationColor;
            }            
        } 
    }else if(led == E_LL_LED_REAR){
        for(int i = 0; i < LL_ANIMATION_FRAME_NUM; i+=2){ 
            for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
                gtPanelPara.animationFramesRear_flash[i][j] = AnimationColor;
            }
        }    
    }     
}

//turn left
void LL_LED_Panel_TurnLeft_Flash_Animation(E_LL_LED led)
{
    //12 frames -- 6frames up : 6frames off cycle:4
    if(led == E_LL_LED_FRONT){
        for(int i=0; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=LL_LED_MATRIX_WIDTH/2; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] =  WS2812_COLOR_WHITE_100;
            }
        }
        for(int i=0; i<6; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH/2; j++){
                gtPanelPara.animationFramesFront_flash[i][j] = WS2812_COLOR_TURNSIGNAL; 
                gtPanelPara.animationFramesFront_flash[i+12][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesFront_flash[i+24][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesFront_flash[i+36][j] = WS2812_COLOR_TURNSIGNAL;
            }            
        }
    }else if(led == E_LL_LED_REAR){
        for(int i=0; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH/2; j++){
                gtPanelPara.animationFramesRear_flash[i][j] =  WS2812_COLOR_RED_100;
            }
        }
        for(int i=0; i<6; i++){
            for(int j=LL_LED_MATRIX_WIDTH/2; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] = WS2812_COLOR_TURNSIGNAL; 
                gtPanelPara.animationFramesRear_flash[i+12][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesRear_flash[i+24][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesRear_flash[i+36][j] = WS2812_COLOR_TURNSIGNAL;
            }            
        }           
    }      
}

//turn right
void LL_LED_Panel_TurnRight_Flash_Animation(E_LL_LED led)
{
    //12 frames -- 6frames up : 6frames off cycle:4
    if(led == E_LL_LED_FRONT){
        for(int i=0; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH/2; j++){
                gtPanelPara.animationFramesFront_flash[i][j] =  WS2812_COLOR_WHITE_100;
            }
        }
        for(int i=0; i<6; i++){
            for(int j=LL_LED_MATRIX_WIDTH/2; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] = WS2812_COLOR_TURNSIGNAL; 
                gtPanelPara.animationFramesFront_flash[i+12][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesFront_flash[i+24][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesFront_flash[i+36][j] = WS2812_COLOR_TURNSIGNAL;
            }            
        }
    }else if(led == E_LL_LED_REAR){
        for(int i=0; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=LL_LED_MATRIX_WIDTH/2; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] =  WS2812_COLOR_RED_100;
            }
        }
        for(int i=0; i<6; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH/2; j++){
                gtPanelPara.animationFramesRear_flash[i][j] = WS2812_COLOR_TURNSIGNAL; 
                gtPanelPara.animationFramesRear_flash[i+12][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesRear_flash[i+24][j] = WS2812_COLOR_TURNSIGNAL;
                gtPanelPara.animationFramesRear_flash[i+36][j] = WS2812_COLOR_TURNSIGNAL;
            }            
        }           
    }      
}

//half blink
void LL_LED_Panel_HalfBlink_Animation(E_LL_LED led)
{
    if(led == E_LL_LED_FRONT){     
        for(int i=0; i<LL_ANIMATION_FRAME_NUM/2; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                    if(AnimationFrame_HalfBlink_Front[0].color_code[j] != 0){
                        gtPanelPara.animationFramesFront_flash[i][j] =  WS2812_COLOR_WHITE;
                    }
            }
        }
        for(int i=LL_ANIMATION_FRAME_NUM/2; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] =  WS2812_COLOR_WHITE;
            }
        }   
    }else if(led == E_LL_LED_REAR){     
        for(int i=0; i<LL_ANIMATION_FRAME_NUM/2; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                    if(AnimationFrame_HalfBlink_Rear[0].color_code[j] != 0){
                        gtPanelPara.animationFramesRear_flash[i][j] =  WS2812_COLOR_RED_80;
                    }
            }
        }   
        for(int i=LL_ANIMATION_FRAME_NUM/2; i<LL_ANIMATION_FRAME_NUM; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] =  WS2812_COLOR_RED_80;
            }
        }           
    }      
}

//alarm
uint8_t flash_timeslot = 0;
void LL_LED_Panel_AlarmAnimation(uint32_t AnimationColor, E_LL_LED led)
{
    unsigned long total_bar_num = 12;         
    unsigned long total_timeslot_num = 48;    //1/48
    unsigned long color_buffer[2];
    
    #define LL_FLASH_PATTERN_FAST_FLASH 0x00AA0AA0
    
    unsigned long setting_of_all_bar = LL_FLASH_PATTERN_FAST_FLASH;

    for (unsigned long timeslot = 0; timeslot < total_timeslot_num; timeslot++)
    {
        unsigned long index_of_now_bar = (timeslot * total_bar_num) / total_timeslot_num;
        unsigned long index_in_bar_of_now_timeslot = timeslot % (total_timeslot_num / total_bar_num);

        unsigned long setting_of_now_bar = GET_BAR_STATE(setting_of_all_bar, index_of_now_bar);

        unsigned long need_ON = 0;
        switch (setting_of_now_bar)
        {
        case 0: // OFF
            need_ON = 0;
            break;
        case 1:
        case 2: // half blink (1/48)
            if (0 == index_in_bar_of_now_timeslot){need_ON = 1;}
            break;
        case 3: // ON
            need_ON = 1;
            break;
        default:
            need_ON = 0;
            break;
        }
        if(need_ON){
            if(led == E_LL_LED_FRONT){
                if(flash_timeslot){flash_timeslot = !flash_timeslot; color_buffer[0] =  WS2812_COLOR_BLUE; color_buffer[1] =  WS2812_COLOR_RED;}
                else{flash_timeslot = !flash_timeslot; color_buffer[0] =  WS2812_COLOR_RED; color_buffer[1] =  WS2812_COLOR_BLUE;}
                for (unsigned long led_index = 0; led_index < LL_LED_MATRIX_WIDTH/2; led_index++)
                {
                    gtPanelPara.animationFramesFront_flash[timeslot][led_index] = color_buffer[0];
                }  
                for (unsigned long led_index = LL_LED_MATRIX_WIDTH/2; led_index < LL_LED_MATRIX_WIDTH; led_index++)
                {
                    gtPanelPara.animationFramesFront_flash[timeslot][led_index] = color_buffer[1];
                }                 
            }else{
                if(flash_timeslot){flash_timeslot = !flash_timeslot; color_buffer[0] =  WS2812_COLOR_RED; color_buffer[1] =  WS2812_COLOR_BLUE;}
                else{flash_timeslot = !flash_timeslot; color_buffer[0] =  WS2812_COLOR_BLUE; color_buffer[1] =  WS2812_COLOR_RED;}
                for (unsigned long led_index = 0; led_index < LL_LED_MATRIX_WIDTH/2; led_index++)
                {
                    gtPanelPara.animationFramesRear_flash[timeslot][led_index] = color_buffer[0];
                }  
                for (unsigned long led_index = LL_LED_MATRIX_WIDTH/2; led_index < LL_LED_MATRIX_WIDTH; led_index++)
                {
                    gtPanelPara.animationFramesRear_flash[timeslot][led_index] = color_buffer[1];
                }                 
            }
        }
    }
}

//Breathing
void LL_LED_Panel_BreathingAnimation(uint32_t AnimationColor, E_LL_LED led)
{
    //48 frames -- 20frames up : 20frames down :  8frames off
    if(led == E_LL_LED_FRONT){
        for(int i=0; i<20; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] = AnimationColor*(i+1); 
            }
        }
        for(int i=20; i<40; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] = AnimationColor*(40-i); 
            }            
        }
        for(int i=40; i<48; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesFront_flash[i][j] = WS2812_COLOR_BLACK; 
            }
        } 
    }else if(led == E_LL_LED_REAR){
        for(int i=0; i<20; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] = AnimationColor*(i+1); 
            }
        }
        for(int i=20; i<40; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] = AnimationColor*(40-i); 
            }
        }
        for(int i=40; i<48; i++){
            for(int j=0; j<LL_LED_MATRIX_WIDTH; j++){
                gtPanelPara.animationFramesRear_flash[i][j] = WS2812_COLOR_BLACK; 
            }
        }            
    }     
}

//Power Off
uint8_t LL_LED_Panel_PowerOffAnimation(void)
{
    #define POWEROFF_STEP_1_FRAME 3
    #define POWEROFF_STEP_2_FRAME 12
    unsigned char batLevel = LL_Battery_Level();
    unsigned char displayFrameNum;
    unsigned long displayColor;
    
    if(batLevel > 80){         displayFrameNum = 14;displayColor = WS2812_COLOR_GREEN;   }
    else if(batLevel > 60){    displayFrameNum = 11;displayColor = WS2812_COLOR_ORANGE;  } 
    else if(batLevel > 40){    displayFrameNum = 9;displayColor = WS2812_COLOR_ORANGE;   } 
    else if(batLevel > 20){    displayFrameNum = 6;displayColor = WS2812_COLOR_RED;      }
    else{                      displayFrameNum = 3;displayColor = WS2812_COLOR_RED;      } 
    
    //3 frames close
    //
    for(int i=0; i<displayFrameNum; i++){
        for(int j=LL_LED_MATRIX_WIDTH-1; j>=LL_LED_MATRIX_WIDTH-(i*2) && j>= 0;j--){
            gtPanelPara.animationFramesRear_flash[i+POWEROFF_STEP_1_FRAME][j] = displayColor; 
        }
    }
    
    //6 frames solid
    for(int i=0; i<POWEROFF_STEP_2_FRAME; i++){
        for(int j=LL_LED_MATRIX_WIDTH-1; j>= LL_LED_MATRIX_WIDTH-(displayFrameNum*2) && j>= 0;j--){
            gtPanelPara.animationFramesRear_flash[i+POWEROFF_STEP_1_FRAME+displayFrameNum][j] = displayColor; 
        }
    }        
        
    return (POWEROFF_STEP_1_FRAME+displayFrameNum+POWEROFF_STEP_2_FRAME);
}

//prodcution test
void LL_LED_Panel_Producting_Animation(void)
{
		#define TEST_COLOR_NUM  4
		unsigned long animation_color_production_test_code[TEST_COLOR_NUM] = { WS2812_COLOR_WHITE_100, WS2812_COLOR_RED_100, WS2812_COLOR_GREEN, WS2812_COLOR_BLUE };
		for(int i=0; i<TEST_COLOR_NUM; i++){
			for(int j= 12*i; j<12*(i+1); j++){
				for(int k=0; k<LL_LED_MATRIX_WIDTH; k++){
					gtPanelPara.animationFramesFront_flash[j][k] =  animation_color_production_test_code[i];
					gtPanelPara.animationFramesRear_flash[j][k] =  animation_color_production_test_code[i];
				}
			}
		}
}
void LL_Drv_Ws2812_SetFrontAnimation(E_Ws2812_Animation_Type AnimationType)
{
    const T_FRAME__RGB_FORMAT *selectedAnimationFrontFrames = NULL;  

    T_FRAME__RGB_FORMAT ConvertedFrontFrames[LL_ANIMATION_FRAME_NUM] = {0};
    
    memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));
    
    switch(AnimationType)
    {
        case LED_ANIMATION_SOLID_MODE:
        case LED_ANIMATION_SLOWFLASH_MODE:
        case LED_ANIMATION_FASTFLASH_MODE:
            LL_Drv_Ws2812_SyncWithClock(E_LL_LED_FRONT);
            return;
        case LED_ANIMATION_HALFBLINK_MODE:
            LL_LED_Panel_HalfBlink_Animation(E_LL_LED_FRONT);
            return; 
        case LED_ANIMATION_DOUBLEWAVE_MODE:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_DoubleWave_Front) / sizeof(AnimationFrame_DoubleWave_Front[0]);   
            LL_Convert_Frames(AnimationFrame_DoubleWave_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;      
            break;   
        case LED_ANIMATION_RICOCHET_MODE:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_Ricochet_Front) / sizeof(AnimationFrame_Ricochet_Front[0]);   
            LL_Convert_Frames(AnimationFrame_Ricochet_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;      
            break;   
        case LED_ANIMATION_ALARM_MODE:
            LL_LED_Panel_AlarmAnimation(WS2812_COLOR_NONEED, E_LL_LED_FRONT);
            return; 
        case LED_ANIMATION_BREATHING_MODE:
            LL_LED_Panel_BreathingAnimation(WS2812_COLOR_WHITE_5, E_LL_LED_FRONT);
            return;           
        case LED_ANIMATION_POWERON_LEVEL_1:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_PowerOn_Level_1_Front) / sizeof(AnimationFrame_PowerOn_Level_1_Front[0]);  
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_1_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames, E_LL_LED_FRONT, glPowerOnDisplayNum);        
            selectedAnimationFrontFrames = ConvertedFrontFrames;     
            break;
        case LED_ANIMATION_POWERON_LEVEL_2:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_PowerOn_Level_2_Front) / sizeof(AnimationFrame_PowerOn_Level_2_Front[0]);  
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_2_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames, E_LL_LED_FRONT, glPowerOnDisplayNum);        
            selectedAnimationFrontFrames = ConvertedFrontFrames;     
            break;
        case LED_ANIMATION_POWERON_LEVEL_3:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_PowerOn_Level_3_Front) / sizeof(AnimationFrame_PowerOn_Level_3_Front[0]);  
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_3_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames, E_LL_LED_FRONT, glPowerOnDisplayNum);        
            selectedAnimationFrontFrames = ConvertedFrontFrames;     
            break;
        case LED_ANIMATION_POWERON_LEVEL_4:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_PowerOn_Level_4_Front) / sizeof(AnimationFrame_PowerOn_Level_4_Front[0]);  
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_4_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames, E_LL_LED_FRONT, glPowerOnDisplayNum);        
            selectedAnimationFrontFrames = ConvertedFrontFrames;     
            break;      
        case LED_ANIMATION_POWEROFF:   
            gtPanelPara.gulWs2812FrameFrontTotal = LL_ANIMATION_FRAME_NUM;
            memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));
            return;         
        case LED_ANIMATION_TURNLEFT_SIGNAL:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_TurnLeftSignal_Front) / sizeof(AnimationFrame_TurnLeftSignal_Front[0]);   
            LL_Convert_Frames(AnimationFrame_TurnLeftSignal_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;    
            break;
//            selectedAnimationFrontFrames = AnimationFrame_TurnLeftSignal;
//            //gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_TurnLeftSignal) / sizeof(AnimationFrame_TurnLeftSignal[0]);
//            break;
        case LED_ANIMATION_TURNRIGHT_SIGNAL:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_TurnRightSignal_Front) / sizeof(AnimationFrame_TurnRightSignal_Front[0]);   
            LL_Convert_Frames(AnimationFrame_TurnRightSignal_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;    
            break;
        case LED_ANIMATION_TURNLEFT_FLASH_SIGNAL:
            LL_LED_Panel_TurnLeft_Flash_Animation(E_LL_LED_FRONT);        
            return;
        case LED_ANIMATION_TURNRIGHT_FLASH_SIGNAL:
            LL_LED_Panel_TurnRight_Flash_Animation(E_LL_LED_FRONT);        
            return;
        case LED_ANIMATION_PAIRING_MODE:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_PairingMode_Front) / sizeof(AnimationFrame_PairingMode_Front[0]);   
            LL_Convert_Frames(AnimationFrame_PairingMode_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;     
            break;    
        case LED_ANIMATION_BRAKE_SIGNAL_SOLID:
            gtPanelPara.gulWs2812FrameFrontTotal = sizeof(AnimationFrame_BrakeSignal_Front) / sizeof(AnimationFrame_BrakeSignal_Front[0]);   
            LL_Convert_Frames(AnimationFrame_BrakeSignal_Front, gtPanelPara.gulWs2812FrameFrontTotal, ConvertedFrontFrames);
            selectedAnimationFrontFrames = ConvertedFrontFrames;    
            break; 
				case LED_ANIMATION_PRODUCT_TESTING:
            LL_LED_Panel_Producting_Animation();        
            return;						
        default:
            break;
    }
    if(selectedAnimationFrontFrames != NULL){
        for (int i = 0; i < LL_ANIMATION_FRAME_NUM; i++) {
            for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
                gtPanelPara.animationFramesFront_flash[i][j] = selectedAnimationFrontFrames[i].data[j];
            }
        }
    }
}

void LL_Drv_Ws2812_SetRearAnimation(E_Ws2812_Animation_Type AnimationType)
{
    const T_FRAME__RGB_FORMAT *selectedAnimationRearFrames = NULL;  
    T_FRAME__RGB_FORMAT ConvertedRearFrames[LL_ANIMATION_FRAME_NUM] = {0};
    
    
    memset(gtPanelPara.animationFramesRear_flash, 0,sizeof(gtPanelPara.animationFramesRear_flash));
    switch(AnimationType)
    {
        case LED_ANIMATION_SOLID_MODE:
        case LED_ANIMATION_SLOWFLASH_MODE:
        case LED_ANIMATION_FASTFLASH_MODE:
            gtPanelPara.gulWs2812FrameRearTotal = 48;
            LL_Drv_Ws2812_SyncWithClock(E_LL_LED_REAR);
            return;
        case LED_ANIMATION_HALFBLINK_MODE:
            gtPanelPara.gulWs2812FrameRearTotal = 48;
            LL_LED_Panel_HalfBlink_Animation(E_LL_LED_REAR);
            return;  
        case LED_ANIMATION_DOUBLEWAVE_MODE:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_DoubleWave_Rear) / sizeof(AnimationFrame_DoubleWave_Rear[0]);   
            LL_Convert_Frames(AnimationFrame_DoubleWave_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;     
            break;       
        case LED_ANIMATION_RICOCHET_MODE:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Ricochet_Rear) / sizeof(AnimationFrame_Ricochet_Rear[0]);   
            LL_Convert_Frames(AnimationFrame_Ricochet_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;     
            break;
        case LED_ANIMATION_ALARM_MODE:
            gtPanelPara.gulWs2812FrameRearTotal = 16;
            LL_LED_Panel_AlarmAnimation(WS2812_COLOR_NONEED, E_LL_LED_REAR);
            return;  
        case LED_ANIMATION_BREATHING_MODE:
            LL_LED_Panel_BreathingAnimation(WS2812_COLOR_RED_5, E_LL_LED_REAR);
            return;     
        case LED_ANIMATION_POWERON_LEVEL_1:            
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_PowerOn_Level_1_Rear) / sizeof(AnimationFrame_PowerOn_Level_1_Rear[0]); 
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_1_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames, E_LL_LED_REAR, glPowerOnDisplayNum);        
            selectedAnimationRearFrames = ConvertedRearFrames;     
            gtPanelPara.gulWs2812FrameRearTotal = LL_ANIMATION_FRAME_NUM;  //in order to playoff animation
            break;  
        case LED_ANIMATION_POWERON_LEVEL_2:            
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_PowerOn_Level_2_Rear) / sizeof(AnimationFrame_PowerOn_Level_2_Rear[0]); 
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_2_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames, E_LL_LED_REAR, glPowerOnDisplayNum);        
            selectedAnimationRearFrames = ConvertedRearFrames; 
            gtPanelPara.gulWs2812FrameRearTotal = LL_ANIMATION_FRAME_NUM; 
            break;
        case LED_ANIMATION_POWERON_LEVEL_3:            
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_PowerOn_Level_3_Rear) / sizeof(AnimationFrame_PowerOn_Level_3_Rear[0]); 
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_3_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames, E_LL_LED_REAR, glPowerOnDisplayNum);        
            selectedAnimationRearFrames = ConvertedRearFrames;     
            gtPanelPara.gulWs2812FrameRearTotal = LL_ANIMATION_FRAME_NUM; 
            break;
        case LED_ANIMATION_POWERON_LEVEL_4:            
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_PowerOn_Level_4_Rear) / sizeof(AnimationFrame_PowerOn_Level_4_Rear[0]); 
            LL_Convert_Frames_PowerOn(AnimationFrame_PowerOn_Level_4_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames, E_LL_LED_REAR, glPowerOnDisplayNum);        
            selectedAnimationRearFrames = ConvertedRearFrames; 
            gtPanelPara.gulWs2812FrameRearTotal = LL_ANIMATION_FRAME_NUM;   
            break;
        case LED_ANIMATION_POWEROFF:
            memset(gtPanelPara.animationFramesRear_flash, 0x00,sizeof(gtPanelPara.animationFramesRear_flash));
            gtPanelPara.gulWs2812FrameRearTotal = LL_LED_Panel_PowerOffAnimation();
            return; 
        case LED_ANIMATION_TURNLEFT_SIGNAL:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_TurnLeftSignal_Rear) / sizeof(AnimationFrame_TurnLeftSignal_Rear[0]);   
            LL_Convert_Frames(AnimationFrame_TurnLeftSignal_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;
        case LED_ANIMATION_TURNRIGHT_SIGNAL:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_TurnRightSignal_Rear) / sizeof(AnimationFrame_TurnRightSignal_Rear[0]);   
            LL_Convert_Frames(AnimationFrame_TurnRightSignal_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;
        case LED_ANIMATION_TURNLEFT_FLASH_SIGNAL:
            LL_LED_Panel_TurnLeft_Flash_Animation(E_LL_LED_REAR);        
            return;
        case LED_ANIMATION_TURNRIGHT_FLASH_SIGNAL:
            LL_LED_Panel_TurnRight_Flash_Animation(E_LL_LED_REAR);        
            return;
        case LED_ANIMATION_PAIRING_MODE:            
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_PairingMode_Rear) / sizeof(AnimationFrame_PairingMode_Rear[0]);   
            LL_Convert_Frames(AnimationFrame_PairingMode_Rear, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;     
            break;  
        
        case LED_ANIMATION_BRAKE_SIGNAL_SOLID:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_BrakeSignal_Rear_Solid) / sizeof(AnimationFrame_BrakeSignal_Rear_Solid[0]);   
            LL_Convert_Frames(AnimationFrame_BrakeSignal_Rear_Solid, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break; 
        case LED_ANIMATION_BRAKE_SIGNAL_SHARPFLASH:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_BrakeSignal_Rear_SharpFlash) / sizeof(AnimationFrame_BrakeSignal_Rear_SharpFlash[0]);   
            LL_Convert_Frames(AnimationFrame_BrakeSignal_Rear_SharpFlash, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break; 
        case LED_ANIMATION_CHARGING:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging) / sizeof(AnimationFrame_Charging[0]);   
            LL_Convert_Frames_Charging(AnimationFrame_Charging, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break; 
        case LED_ANIMATION_CHARING_LEVEL_0:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_0) / sizeof(AnimationFrame_Charging_Level_0[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_0, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;   
        case LED_ANIMATION_CHARING_LEVEL_1:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_1) / sizeof(AnimationFrame_Charging_Level_1[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_1, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;      
        case LED_ANIMATION_CHARING_LEVEL_2:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_2) / sizeof(AnimationFrame_Charging_Level_2[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_2, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;      
        case LED_ANIMATION_CHARING_LEVEL_3:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_3) / sizeof(AnimationFrame_Charging_Level_3[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_3, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;      
        case LED_ANIMATION_CHARING_LEVEL_4:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_4) / sizeof(AnimationFrame_Charging_Level_4[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_4, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break; 
        case LED_ANIMATION_CHARING_LEVEL_5:
            gtPanelPara.gulWs2812FrameRearTotal = sizeof(AnimationFrame_Charging_Level_5) / sizeof(AnimationFrame_Charging_Level_5[0]);   
            LL_Convert_Frames(AnimationFrame_Charging_Level_5, gtPanelPara.gulWs2812FrameRearTotal, ConvertedRearFrames);
            selectedAnimationRearFrames = ConvertedRearFrames;    
            break;          
				case LED_ANIMATION_PRODUCT_TESTING:
            LL_LED_Panel_Producting_Animation();        
            return;	
        default:
            break;
    }
    
    if(selectedAnimationRearFrames != NULL){    
        for (int i = 0; i < LL_ANIMATION_FRAME_NUM; i++) {
            for (int j = 0; j < LL_LED_MATRIX_WIDTH; j++) {
                gtPanelPara.animationFramesRear_flash[i][j] = selectedAnimationRearFrames[i].data[j];
            }
        }
    }      
}


bool LL_Find_Color_Position(unsigned long ColorDisk, unsigned char *ColorType, unsigned char *BrightnessLevel) {
    for (uint8_t i = 0; i < LL_COLOR_TYPE_NUM; i++) {
        for (uint8_t j = 0; j < LL_COLOR_BRIGHTNESS_LEVEL; j++) {
            if (Animation_Custom_Color_Disk[i][j] == ColorDisk) {
                *ColorType = i;
                
                if(j==0){*BrightnessLevel = 0;}
                else if(j==1){*BrightnessLevel = 30;}
                else if(j==2){*BrightnessLevel = 50;}
                else if(j==3){*BrightnessLevel = 70;}
                else if(j==4){*BrightnessLevel = 90;}
                else if(j==5){*BrightnessLevel = 0xFF;}
                return true;
            }
        }
    }
    return false; 
}

bool LL_Get_Color_Value_Via_Individual_Brightness(unsigned char Brightness, unsigned long *light_color) 
{
    uint8_t light_type_found = 0;
    if(Brightness == 0xFF){Brightness = 5;}
    else if(Brightness > 100){Brightness = 4;} //ERROR
    else if(Brightness == 100){Brightness = 4;} //100% not max
    else{Brightness /= 20; }

    for (uint8_t i = 0; i < LL_COLOR_TYPE_NUM; i++) {
        for (uint8_t j = 0; j < LL_COLOR_BRIGHTNESS_LEVEL; j++) {
            if (Animation_Custom_Color_Disk[i][j] == *light_color) {
                light_type_found = i;
            }
        }
    }
    
    *light_color = Animation_Custom_Color_Disk[light_type_found][Brightness];
    return true;
}

bool LL_Get_Color_Value_Via_Brightness(unsigned char Brightness, unsigned long *front_color, unsigned long *rear_color) 
{
    uint8_t front_type_found = 0;
    uint8_t rear_type_found = 0;
    if(Brightness == 0xFF){Brightness = 5;}
    else if(Brightness > 100){Brightness = 4;} //ERROR
    else if(Brightness == 100){Brightness = 4;} //100% not max
    else{Brightness /= 20; }

    for (uint8_t i = 0; i < LL_COLOR_TYPE_NUM; i++) {
        for (uint8_t j = 0; j < LL_COLOR_BRIGHTNESS_LEVEL; j++) {
            if (Animation_Custom_Color_Disk[i][j] == *front_color) {
                front_type_found = i;
            }
            if (Animation_Custom_Color_Disk[i][j] == *rear_color) {
                rear_type_found = i;
            }
        }
    }
    
    *front_color = Animation_Custom_Color_Disk[front_type_found][Brightness];
    *rear_color = Animation_Custom_Color_Disk[rear_type_found][Brightness];
    return true;
}

bool LL_Get_Color_Value_Via_Color(unsigned char colorType, unsigned long * color) 
{
    if(colorType >= LL_COLOR_TYPE_NUM ){ colorType = LL_COLOR_TYPE_NUM - 1;}
    uint8_t find_brightness = 0;
    for (uint8_t i = 0; i < LL_COLOR_TYPE_NUM; i++) {
        for (uint8_t j = 0; j < LL_COLOR_BRIGHTNESS_LEVEL; j++) {
            if (Animation_Custom_Color_Disk[i][j] == *color) {
                find_brightness = j;
            }
        }
    }
    
    *color = Animation_Custom_Color_Disk[colorType][find_brightness];
    return true;
}

#if 0
//hash: need more ram!
#define COLOR_TABLE_SIZE          (LL_COLOR_TYPE_NUM * LL_COLOR_BRIGHTNESS_LEVEL)
typedef struct {
    uint32_t key;
    uint8_t type;
    uint8_t level;
    bool valid;
} ColorLookupEntry;

static ColorLookupEntry color_lookup_table[COLOR_TABLE_SIZE];

static uint32_t color_hash(uint32_t key) {
    return (key ^ (key >> 8) ^ (key >> 16)) % COLOR_TABLE_SIZE;
}

void build_color_lookup_table() {
    for (uint8_t type = 0; type < LL_COLOR_TYPE_NUM; ++type) {
        for (uint8_t level = 0; level < LL_COLOR_BRIGHTNESS_LEVEL; ++level) {
            uint32_t val = Animation_Custom_Color_DISK[type][level];
            uint32_t idx = color_hash(val);
            while (color_lookup_table[idx].valid) {
                idx = (idx + 1) % COLOR_TABLE_SIZE;
            }
            color_lookup_table[idx].key = val;
            color_lookup_table[idx].type = type;
            color_lookup_table[idx].level = level;
            color_lookup_table[idx].valid = true;
        }
    }
}

bool find_color_position(uint32_t color_disk, uint8_t *type, uint8_t *level) {
    uint32_t idx = color_hash(color_disk);
    for (uint32_t i = 0; i < COLOR_TABLE_SIZE; ++i) {
        uint32_t try = (idx + i) % COLOR_TABLE_SIZE;
        if (!color_lookup_table[try].valid) break;
        if (color_lookup_table[try].key == color_disk) {
            *type = color_lookup_table[try].type;
            *level = color_lookup_table[try].level;
            return true;
        }
    }
    return false;
}
#endif
