/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Platform.h"
#include "LL_Common.h"
//#include "LL_SysMode.h"

unsigned char gulCharging = 0;
unsigned long LL_Battery_Charging__NotOnlyChargingMode_ButAlsoChargingIndeed(void) {
    return LL_Battery__isCharging();
}

unsigned long LL_Battery__isCharging(void) {
    unsigned long lumos5V         = LL_GPIO_InputRead(0, LL_PIN_CHARGING_5V);
    return (LL_PIN_Y__CHARGING_5V == lumos5V);
}

void LL_Battery_Charging__Init(void) {
    LL_GPIO_InputCfg(0, LL_PIN_CHARGING_5V,     LL_PULLMODE_CHARGING_5V,    LL_GPIO_TRIGGER_NONE);
}

void LL_Battery_Charging_Animation_Init(void)
{
    LL_GPIO_OutputWrite(0, LL_PIN_CHARGING_POWER, LL_PIN_Y__CHARGING_POWER); 
    LL_GPIO_OutputWrite(0, LL_PIN_LED_POWER_ENABLE, LL_PIN_Y__LED_POWER_ENABLE);	
		LL_Key_Init_With_No_Trigger();
		memset(gtPanelPara.animationFramesFront_flash, 0,sizeof(gtPanelPara.animationFramesFront_flash));

    glPowerOnAnimationPlayOff = true; //don't need play at first	
		
		LL_BLE_Adv_stop();
		//Charging Animation
    LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_CHARGING);
	
		gtPara.ulNeedCharge = 0;
		gulFlashStoreNeeded = 1;
}

unsigned char sgulChargingDisplayStep = 0,sgulChargingGeneralStep = 0;
unsigned long ulTimerCntBatteryUpdateWhenCharging;
void LL_Battery_Charging_Animation_Loop(void) {
    unsigned long ulSample_1,ulSample_2;
    unsigned char glBatRank;
    unsigned long ulTimerFreqUpdateAnimationCnt = 1800;
		T_LL_KEY_EVT tKeyEvt;
	
    {
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
								gulCharging = 0;
                gulFlashStoreNeeded = 1;
								LL_Drv_Ws2812_SetFrontAnimation(LED_ANIMATION_BLACK_SCREEN);
								LL_Drv_Ws2812_SetRearAnimation(LED_ANIMATION_BLACK_SCREEN);    
                LL_Helmet_ChangeStateTo_OFF();          
            }
        }
       
    }      
}
