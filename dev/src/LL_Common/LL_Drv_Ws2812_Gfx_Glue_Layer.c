
#include <stdbool.h>
#include <stdint.h>
#include "nrf_assert.h"
#include "nrf_gpio.h"
#include "sdk_errors.h"
#include "LL_Drv_Ws2812.h"
#include "LL_Drv_Ws2812_Gfx_Glue_Layer.h"
#include "LL_GPIO.h"

extern unsigned long gulWs2812WorkCnt;
extern unsigned long gulWs2812WorkCntTop;
ret_code_t LL_Drv_Ws2812_Reset(void)
{
    //reset
	uint8_t i = 0;
	uint32_t err_code;
	//reset
	for(i = 0; i < LED_MATRIX_WIDTH; i++)
	{
		err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_BLACK);
		if(err_code) return err_code;				
	}  
    return NRF_SUCCESS;
}

//mode: full beam FronWhite & RearRed
void LL_Drv_Ws2812_Set_Solid_Mode(void)
{
	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, WS2812_COLOR_WHITE_20);
	LL_Convert_Rgb_To_Pwm_Sequence_Channel0();  

    //reset
    LL_Drv_Ws2812_Reset();
    
	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, WS2812_COLOR_RED_20);
	LL_Convert_Rgb_To_Pwm_Sequence_Channel1();    
    
    //reset
    LL_Drv_Ws2812_Reset();    
}

//mode: slow flash FronWhite & RearRed
void LL_Drv_Ws2812_Set_Slow_Flash_Mode(uint8_t timecnt)
{
    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_WHITE_20);
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, timecnt+1, 1, WS2812_COLOR_WHITE_20);  
        
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();

    //reset
    LL_Drv_Ws2812_Reset();    

    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_RED_20);  
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, timecnt+1, 1, WS2812_COLOR_RED_20);  
    
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    
    //reset
    LL_Drv_Ws2812_Reset();   
}

//mode: turn left
void LL_Drv_Ws2812_TurnLeft(uint8_t timecnt)
{
    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_YELLOW);
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20);  
        
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();

    //reset
    LL_Drv_Ws2812_Reset();    

    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_YELLOW);  
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20);  
    
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();

    //reset
    LL_Drv_Ws2812_Reset();      
}

//mode: turn right
void LL_Drv_Ws2812_TurnRight(uint8_t timecnt)
{
    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(0, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20); 
    
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, 1+timecnt, 1, WS2812_COLOR_YELLOW);  
        
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();

    //reset
    LL_Drv_Ws2812_Reset();    

    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(0, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20);  
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, 1+timecnt, 1, WS2812_COLOR_YELLOW);  
    
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();

    //reset
    LL_Drv_Ws2812_Reset();      
}

//mode:full beam
void LL_Drv_Ws2812_Mode(uint32_t LedType)
{
	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LedType);
	LL_Convert_Rgb_To_Pwm_Sequence();
}

//mode:full beam loop
void LL_Drv_Ws2812_Mode0(uint8_t timecnt)
{

	switch(timecnt)
	{
		case 0:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_RED);
			break;
		case 4:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;
		case 9:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_GREEN);
			break;
		case 14:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;
		case 19:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLUE);
			break;
		case 24:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;
		case 29:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_PURPLE);
			break;
		case 34:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;		
		case 39:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_ANTIQUEWHITE);
			break;
		case 44:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;
		case 49:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_YELLOW);
			break;
		case 54:
			LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
			break;						
	}
}

//mode:ricochet
uint8_t needverse = 0;
ret_code_t LL_Drv_Ws2812_Mode1(uint8_t timecnt)
{
    
    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_WHITE_20);
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, timecnt+1, 1, WS2812_COLOR_WHITE_20);  
        
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();

    //reset
    LL_Drv_Ws2812_Reset();    

    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_RED_20);  
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, timecnt+1, 1, WS2812_COLOR_RED_20);  
    
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    
    //reset
    LL_Drv_Ws2812_Reset(); 
    
        
}

//mode:pulse out
ret_code_t LL_Drv_Ws2812_Mode2(uint8_t timecnt)
{
		uint32_t err_code;
	  

		//convert_rgb_to_pwm_sequence();
		LL_Drv_Ws2812_Reset();
		//right
		err_code = LL_Drv_Ws2812_Pixel_Draw(timecnt, 0, WS2812_COLOR_RED); 
		LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
		
		//reset
		LL_Drv_Ws2812_Reset();
	
		//left
		err_code = LL_Drv_Ws2812_Pixel_Draw(LED_MATRIX_WIDTH-1 - timecnt, 0, WS2812_COLOR_RED); 
		LL_Convert_Rgb_To_Pwm_Sequence_Channel1();		
		
		return err_code;
}

//mode:flux capacitor
ret_code_t LL_Drv_Ws2812_Mode3(uint8_t timecnt)
{
		int i;
		uint32_t err_code;
		
		//reset
		LL_Drv_Ws2812_Reset();
		
		//right
		err_code = LL_Drv_Ws2812_Pixel_Draw(0, 0, WS2812_COLOR_RED);
		err_code = LL_Drv_Ws2812_Pixel_Draw(1, 0, WS2812_COLOR_RED);
		if(timecnt > 1){
			for(i = timecnt; i < (timecnt + 2); i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_RED);
					if(err_code) return err_code;
			}
		}
		LL_Convert_Rgb_To_Pwm_Sequence_Channel0();	

		//reset
		LL_Drv_Ws2812_Reset();
		
		//left
		err_code = LL_Drv_Ws2812_Pixel_Draw(LED_MATRIX_WIDTH-1, 0, WS2812_COLOR_RED);
		err_code = LL_Drv_Ws2812_Pixel_Draw(LED_MATRIX_WIDTH-2, 0, WS2812_COLOR_RED);
		if(timecnt > 1){
			for(i = (LED_MATRIX_WIDTH-2)-timecnt; i < (LED_MATRIX_WIDTH-timecnt); i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_RED);
					if(err_code) return err_code;
			}
		}
		LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
		
		return err_code;
}

//mode:half blink
ret_code_t LL_Drv_Ws2812_Mode4(uint8_t timecnt)
{
		int i;
		uint32_t err_code;
	
		//left
		if(timecnt == 10){
			LL_Drv_Ws2812_Reset();
			for(i = 0; i < LED_MATRIX_WIDTH; i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_WHITE_20);
					if(err_code) return err_code;
			}
			
			for(i = 1; i < LED_MATRIX_WIDTH; i+=2)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_BLACK);
					if(err_code) return err_code;
			}
			LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
			
			LL_Drv_Ws2812_Reset();
			for(i = 0; i < LED_MATRIX_WIDTH; i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_RED);
					if(err_code) return err_code;
			}			
			for(i = 0; i < LED_MATRIX_WIDTH; i+=2)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_BLACK);
					if(err_code) return err_code;
			}			
			LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
		}else if(timecnt == 0){
			for(i = 0; i < LED_MATRIX_WIDTH; i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_WHITE_20);
					if(err_code) return err_code;
			}
			LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
            
            LL_Drv_Ws2812_Reset();
            
			for(i = 0; i < LED_MATRIX_WIDTH; i++)
			{
					err_code = LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_RED);
					if(err_code) return err_code;
			}
			LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
		}		
		
		return err_code;
}

//mode:double wave
ret_code_t LL_Drv_Ws2812_Mode5(uint8_t timecnt)
{
		uint32_t err_code;
		//left
		if(timecnt < 16){  //0/2/4/6/8/10/12/14
			if((timecnt+1) %2 == 0){
				err_code = LL_Drv_Ws2812_Pixel_Draw((((timecnt+1)/2)-1)*2, 0, WS2812_COLOR_RED);
			}
		}else if(timecnt >= 16 && timecnt < 23){
				err_code = LL_Drv_Ws2812_Pixel_Draw((timecnt-15)*2-1, 0, WS2812_COLOR_RED);
		}else{
				LL_Drv_Ws2812_Reset();
		}
		LL_Convert_Rgb_To_Pwm_Sequence();
		

		return err_code;
}

ret_code_t LL_Drv_Ws2812_Mode6(uint8_t timecnt)
{
    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_YELLOW);
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20);  
        
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();

    //reset
    LL_Drv_Ws2812_Reset();    

    //left: 13-0
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop-1-timecnt, 0, timecnt+1, 1, WS2812_COLOR_YELLOW);  
    //right: 14-27
    LL_Drv_Ws2812_Rectangle_Draw(gulWs2812WorkCntTop, 0, gulWs2812WorkCntTop, 1, WS2812_COLOR_RED_20);  
    
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();

    //reset
    LL_Drv_Ws2812_Reset();      
}

uint8_t led_inverse = 0;
void LL_Drv_Ws2812_Mode7(uint8_t timecnt)
{
	#define WS2812_COLOR_LIGHTRED 0x001F0000
	#define WS2812_COLOR_YELLOWWISH 0x00401F00
	if(timecnt == 18){
		if(!led_inverse){
			//left
			LL_Drv_Ws2812_Reset();
			LL_Drv_Ws2812_Rectangle_Draw(4, 0, LED_MATRIX_WIDTH-4, LED_MATRIX_HEIGHT, WS2812_COLOR_LIGHTRED);
			LL_Drv_Ws2812_Pixel_Draw(0, 0, WS2812_COLOR_YELLOW);
			for(uint8_t i =1; i< 4;i++){
				LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_YELLOWWISH);
			}
			LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
			
			//right
			LL_Drv_Ws2812_Reset();
			LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH-4, LED_MATRIX_HEIGHT, WS2812_COLOR_LIGHTRED);
			LL_Drv_Ws2812_Pixel_Draw(LED_MATRIX_WIDTH-1, 0, WS2812_COLOR_YELLOW);
			for(uint8_t i =LED_MATRIX_WIDTH-4; i< LED_MATRIX_WIDTH-1;i++){
				LL_Drv_Ws2812_Pixel_Draw(i, 0, WS2812_COLOR_YELLOWWISH);
			}
			LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
			
			led_inverse =1;
		}else{
			//left
			LL_Drv_Ws2812_Reset();			
			LL_Drv_Ws2812_Rectangle_Draw(4, 0, LED_MATRIX_WIDTH-4, LED_MATRIX_HEIGHT, WS2812_COLOR_LIGHTRED);
			LL_Drv_Ws2812_Rectangle_Draw(0, 0, 4, LED_MATRIX_HEIGHT, WS2812_COLOR_BLACK);
			
			LL_Convert_Rgb_To_Pwm_Sequence_Channel1();

			//right
			LL_Drv_Ws2812_Reset();			
			LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH-4, LED_MATRIX_HEIGHT, WS2812_COLOR_LIGHTRED);		
			LL_Drv_Ws2812_Rectangle_Draw(LED_MATRIX_WIDTH-4, 0, 4, LED_MATRIX_HEIGHT, WS2812_COLOR_BLACK);
			LL_Convert_Rgb_To_Pwm_Sequence_Channel0();			
			
			led_inverse =0;		
		}
	}
	//convert_rgb_to_pwm_sequence();
}

//mode:turn left
void LL_Drv_Ws2812_Mode8(uint32_t LedType,uint8_t timecnt)
{
    if(timecnt < 15){			
        //left
        LL_Drv_Ws2812_Reset();
        
        LL_Drv_Ws2812_Rectangle_Draw(LED_MATRIX_WIDTH-1-timecnt, 0, timecnt+1, LED_MATRIX_HEIGHT, WS2812_COLOR_YELLOW);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();	
    }else{
        LL_Drv_Ws2812_Reset();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    }

    
    LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LedType);
    LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
}

//mode:turn right
void LL_Drv_Ws2812_Mode9(uint32_t LedType,uint8_t timecnt)
{
    if(timecnt < 15){			
        //right
        LL_Drv_Ws2812_Reset();
        
		LL_Drv_Ws2812_Rectangle_Draw(0, 0, timecnt+1, LED_MATRIX_HEIGHT, WS2812_COLOR_YELLOW);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();	
    }else{
        LL_Drv_Ws2812_Reset();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
    }

    
    LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LedType);
    LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
}

//mode:brake light
void LL_Drv_Ws2812_Mode10(uint32_t LedType)
{
	LL_Drv_Ws2812_Rectangle_Draw(0, 0, LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LedType);
	LL_Convert_Rgb_To_Pwm_Sequence();
    
    LL_GPIO_OutputWrite(0, 2, 1);	
    
    LL_GPIO_OutputWrite(0, 9, 1);	
}

//mode:turn left all
void LL_Drv_Ws2812_Mode11(uint32_t LedType,uint8_t timecnt)
{
    if(timecnt < 15){
         LL_Drv_Ws2812_Reset();
         //left
        LL_Drv_Ws2812_Rectangle_Draw(0, 0, timecnt+1, LED_MATRIX_HEIGHT, LedType);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();	        


    }else if(timecnt < 30){
        //right
        LL_Drv_Ws2812_Reset();
        
		LL_Drv_Ws2812_Rectangle_Draw(0, 0, timecnt+1-LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LedType);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();	

    } else {       
        LL_Drv_Ws2812_Reset();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    }
   
}

//mode:turn right all
void LL_Drv_Ws2812_Mode12(uint32_t LedType,uint8_t timecnt)
{
    if(timecnt < 15){			
        //right
        LL_Drv_Ws2812_Reset();
        
        LL_Drv_Ws2812_Rectangle_Draw(LED_MATRIX_WIDTH-1-timecnt, 0, timecnt+1, LED_MATRIX_HEIGHT, WS2812_COLOR_YELLOW);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
    }else if(timecnt < 30){		
     
        //left
        LL_Drv_Ws2812_Reset();
        
        LL_Drv_Ws2812_Rectangle_Draw(LED_MATRIX_WIDTH*2-1-timecnt, 0, timecnt+1-LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, WS2812_COLOR_YELLOW);
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();	        
    }else{
        LL_Drv_Ws2812_Reset();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel0();
        LL_Convert_Rgb_To_Pwm_Sequence_Channel1();
    }
}

//void LL_WS2812_SetFlashPattern(uint8_t glWs2812Mode)
//{
//        if(glWs2812Mode != 1 || glWs2812Mode != 10){
//            LL_GPIO_OutputWrite(0, 2, 0);	
//            LL_GPIO_OutputWrite(0, 9, 0);	           
//        }  
//			switch(glWs2812Mode)
//			{
//				case 0:
//					//LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
//                    LL_Drv_Ws2812_Mode(WS2812_COLOR_RED);
//					break;
//				case 1:
//                    LL_Drv_Ws2812_Mode11(WS2812_COLOR_YELLOW,gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH*2 + 4;
//					break;
//				case 2:
//                    LL_Drv_Ws2812_Mode12(WS2812_COLOR_YELLOW,gulWs2812WorkCnt);
//                    gulWs2812WorkCntTop = LED_MATRIX_WIDTH*2 + 4;
//					//LL_Drv_Ws2812_Mode0(gulWs2812WorkCnt);
//					//gulWs2812WorkCntTop = 60;  //flash 0.25s off 0.25s,6 led types
//					break;
//				case 3:
//					LL_Drv_Ws2812_Mode1(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH-2;
//					break;
//				case 4:
//					LL_Drv_Ws2812_Mode2(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH;
//					break;
//				case 5:
//					LL_Drv_Ws2812_Mode3(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH-1;
//					break;
//				case 6:
//					LL_Drv_Ws2812_Mode4(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = 20;
//					break;
//				case 7:
//					LL_Drv_Ws2812_Mode5(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = 24;
//					break;
//				case 8:
//					LL_Drv_Ws2812_Mode6(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH+4;
//					break;
//				case 9:
//					LL_Drv_Ws2812_Mode7(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = 20;		
//					break;
//                case 10:
//                    LL_Drv_Ws2812_Mode8(WS2812_COLOR_RED, gulWs2812WorkCnt);
//                    gulWs2812WorkCntTop = LED_MATRIX_WIDTH+4;
//					break;
//                case 11:
//                    LL_Drv_Ws2812_Mode9(WS2812_COLOR_RED, gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH+4;
//					break;
//                case 12:
//                    LL_Drv_Ws2812_Mode10(WS2812_COLOR_BLACK);
//					break;
//                case 13:
//                    LL_Drv_Ws2812_Mode11(WS2812_COLOR_YELLOW,gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH*2 + 4;
//					break;
//                case 14:
//                    LL_Drv_Ws2812_Mode12(WS2812_COLOR_YELLOW,gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH*2 + 4;
//					break;
//                case 15:
//                    LL_Drv_Ws2812_Mode(WS2812_COLOR_BLACK);
//					break;
//				case 4:
//                    LL_Drv_Ws2812_Mode(WS2812_COLOR_RED_40);
//					//LL_Drv_Ws2812_Mode2(gulWs2812WorkCnt);
//					//gulWs2812WorkCntTop = LED_MATRIX_WIDTH;
//					break;
//				case 5:
//                    gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2 ;
//                    LL_Drv_Ws2812_Mode1(gulWs2812WorkCnt);
//					break;
//				case 6:
//                    gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2 ;
//                    LL_Drv_Ws2812_Set_Slow_Flash_Mode(gulWs2812WorkCnt);
//					break;
//				case 7:
//                    gulWs2812WorkCntTop = 20;
//					LL_Drv_Ws2812_Mode4(gulWs2812WorkCnt);
//					
//					break;
//				case 8:
//					LL_Drv_Ws2812_Mode5(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = 24;
//					break;
//				case 9:
//					LL_Drv_Ws2812_Mode6(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2;
//					break;
//                case 10:
//					LL_Drv_Ws2812_Mode7(gulWs2812WorkCnt);
//					gulWs2812WorkCntTop = 20;	
//					break;
//                case 11:
//                    LL_Drv_Ws2812_Mode8(WS2812_COLOR_RED, gulWs2812WorkCnt);
//                    gulWs2812WorkCntTop = LED_MATRIX_WIDTH+4;
//					break;
//                case 12:
//                    LL_Drv_Ws2812_Mode10(WS2812_COLOR_BLACK);
//					break;
//			}
//}
void LL_WS2812_SetFlashPattern(uint8_t glWs2812Mode)
{
    switch(glWs2812Mode)
    {
        case 0:
            //LL_Drv_Ws2812_Mode(WS2812_COLOR_RED_20);
            LL_Drv_Ws2812_Set_Solid_Mode();
            break;
        case 1:
            gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2 ;
            LL_Drv_Ws2812_Set_Slow_Flash_Mode(gulWs2812WorkCnt);
            break;
        case 2:
            gulWs2812WorkCntTop = 20;
            LL_Drv_Ws2812_Mode4(gulWs2812WorkCnt);
            break;
        case 3:
            gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2;
            LL_Drv_Ws2812_TurnLeft(gulWs2812WorkCnt);
            break;
        case 4:
            gulWs2812WorkCntTop = LED_MATRIX_WIDTH/2;
            LL_Drv_Ws2812_TurnRight(gulWs2812WorkCnt);
            break;
    }
}
