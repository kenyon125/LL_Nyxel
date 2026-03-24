/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Hardware_cfg.h"
#include "LL_Timer.h"
#include "LL_GPIO.h"
#include "LL_ADC.h"
#include "LL_Battery_Charge.h"
#include "LL_Battery.h"



// Requirement from Kx: 
//              Measure per 20s, average per 2min.
// Implement key point: 
//              HIGHer freq to get both the peak and valley.
//              LONGer window to collect all the cases for average. (Pls avoid the "overflow" of the "total" variable!)
#define LL_BATTERY_SAMPLE_INTERVAL  50      // unit: ms
#define LL_BATTERY_SAMPLE_NUM_MAX   2400    // 2400 samples, which is 2 minutes when 50ms sample rate, and the maximumu of their "total" is 1023*2400 = 0x2576A0, which can be stored in a 32-bit variable.



typedef struct
{
    // temp used in every sample window
    unsigned long ulSampleCnt;
    unsigned long ulMax;
    unsigned long ulMin;
    unsigned long ulTotal;
    // the final measurement
    unsigned long ulFinal;
}T_BatteryMeasure;
static inline void T_BatteryMeasure__ResetTempVar(T_BatteryMeasure *pt) {
    pt->ulSampleCnt     = 0;
    pt->ulMax           = 0;
    pt->ulMin           = 0xFFFFFFFF;
    pt->ulTotal         = 0;
}
static inline void T_BatteryMeasure__Init(T_BatteryMeasure *pt) {
    T_BatteryMeasure__ResetTempVar(pt);
    pt->ulFinal = LL_ADC_SAMPLE_VALUE_NONE;
}
static T_BatteryMeasure gtBatteryMeasure;


void LL_Battery_Init(void) {
    // hardware
    LL_GPIO_OutputCfg(0, LL_PIN__BATT_LVL, LL_PULL__BATT_LVL, LL_PIN_N__BATT_LVL);    
    LL_GPIO_OutputWrite(0, LL_PIN__BATT_LVL, LL_PIN_Y__BATT_LVL);
    // software
    T_BatteryMeasure__Init(&gtBatteryMeasure);
}


static unsigned long sgulAdcSmplCnt = 0x80000000; // make the 1st sample more quickly, because the gulTimerCnt1ms will be 0 at the begining.
static unsigned long sgulAdcSmplSeq = 0;
void LL_Battery_Mainloop(void) {
    unsigned long charging = LL_Battery_Charging__NotOnlyChargingMode_ButAlsoChargingIndeed();
    // start the ADC conversion periodically:    
    if( LL_BATTERY_SAMPLE_INTERVAL < LL_Timer_Elapsed_ms(sgulAdcSmplCnt) ) { sgulAdcSmplCnt = gulTimerCnt1ms;
        LL_ADC_StartSingleConversion();
    }

    // wait ADC hardware sample finish:
    if( sgulAdcSmplSeq == LL_ADC_SampleSeq ) { return; } else { sgulAdcSmplSeq  = LL_ADC_SampleSeq; }
    
    // get the sample
    unsigned long ulSample = LL_ADC_Sample;
    // update statistic
    if(ulSample > gtBatteryMeasure.ulMax) { gtBatteryMeasure.ulMax = ulSample; }
    if(ulSample < gtBatteryMeasure.ulMin) { gtBatteryMeasure.ulMin = ulSample; }
    gtBatteryMeasure.ulTotal += ulSample;
    // if a sample window finished 
    if(gtBatteryMeasure.ulSampleCnt++ >= LL_BATTERY_SAMPLE_NUM_MAX) {
        unsigned long ulFinalOfThisWindow = gtBatteryMeasure.ulTotal / gtBatteryMeasure.ulSampleCnt; // use Average directly
        // check whether the result of this window is reasonable: shoule up/down when charging/discharging
        if(charging) {
            if(ulFinalOfThisWindow > gtBatteryMeasure.ulFinal) { gtBatteryMeasure.ulFinal = ulFinalOfThisWindow; } else { /* discard this window */ }
        } else {
            if(ulFinalOfThisWindow < gtBatteryMeasure.ulFinal) { gtBatteryMeasure.ulFinal = ulFinalOfThisWindow; } else { /* discard this window */ }
        }
        // reset the statistic buf:
        T_BatteryMeasure__ResetTempVar(&gtBatteryMeasure);
    }
}

static unsigned long LL_Battery_Sample(void) {
    if(gtBatteryMeasure.ulFinal != LL_ADC_SAMPLE_VALUE_NONE) { // there is a final
        return gtBatteryMeasure.ulFinal;
    } else { // there is no final calculated yet
        if(gtBatteryMeasure.ulSampleCnt != 0) { // there are some samples already
            unsigned long tempFinal =  gtBatteryMeasure.ulTotal / gtBatteryMeasure.ulSampleCnt;  // calculate a average
            gtBatteryMeasure.ulFinal = tempFinal; // use this temp directly to avoid "the Final are diff if helmet on then off in a sample window"
            return gtBatteryMeasure.ulFinal;
        } else { // there is even no sample yet
            return LL_ADC_SAMPLE_VALUE_NONE;
        }
    }
}

static unsigned long ADC_to_BattLvl(
    unsigned long ulADC, 
    unsigned long *tableADC_descending, 
    unsigned long *tableBattLvl_descending, 
    unsigned long table_size) 
{
    for(int i = 0; i < table_size; i++) {
        if(ulADC > tableADC_descending[i]) { // if found the section
            // calculate the battery level by ADC value
            if(i == 0) { // if the ADC value is larger than the whole table
                return tableBattLvl_descending[0]; // return the 1st noe (largest)
            } else {
                unsigned long maxADC = tableADC_descending[i-1], maxBL = tableBattLvl_descending[i-1];
                unsigned long minADC = tableADC_descending[i],   minBL = tableBattLvl_descending[i];
                unsigned long ulBattLvl = ( ulADC - minADC) * (maxBL - minBL)
                        / (maxADC - minADC)
                        + minBL; if(ulBattLvl > maxBL) { ulBattLvl = maxBL; }
                return ulBattLvl;
            }
        } else { continue; }
    }
    // the ADC value is smaller than the whole table
    return tableBattLvl_descending[table_size-1];  // return the last noe (smallest)
}
unsigned long LL_Battery_Level(void) {
    unsigned long ulADC = LL_Battery_Sample();
    if(LL_ADC_SAMPLE_VALUE_NONE != ulADC) {
        if(LL_Battery_Charging__NotOnlyChargingMode_ButAlsoChargingIndeed()) {
            unsigned long tableADC_descending[    POINT_NUM_WHEN_CHARGING] = LL_BATTERY__TABLE_ADC_DESCENDING_WHEN_CHARGING;
            unsigned long tableBattLvl_descending[POINT_NUM_WHEN_CHARGING] = LL_BATTERY__TABLE_LVL_DESCENDING_WHEN_CHARGING;
            return ADC_to_BattLvl(ulADC, tableADC_descending, tableBattLvl_descending, POINT_NUM_WHEN_CHARGING);
        } else {
            unsigned long tableADC_descending[    POINT_NUM_WHEN_DISCHARGING] = LL_BATTERY__TABLE_ADC_DESCENDING_WHEN_DISCHARGING;
            unsigned long tableBattLvl_descending[POINT_NUM_WHEN_DISCHARGING] = LL_BATTERY__TABLE_LVL_DESCENDING_WHEN_DISCHARGING;
            return ADC_to_BattLvl(ulADC, tableADC_descending, tableBattLvl_descending, POINT_NUM_WHEN_DISCHARGING);
        }
    } else { return LL_BATTERY_LEVEL_NONE; }
}


