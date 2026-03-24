/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_HARDWARE_CFG_V0P0_H
#define _LL_HARDWARE_CFG_V0P0_H



#define PIN_INPUT_NO_USE    ?//13
#define PIN_OUTPUT_NO_USE   ?//14

#define LL_PIN__ADAPTER_BOARD_DETECT        21
#define LL_PULL__ADAPTER_BOARD_DETECT       LL_GPIO_PULL_NONE
#define LL_PIN_Y__ADAPTER_BOARD_DETECT      1
#define LL_PIN_N__ADAPTER_BOARD_DETECT      0

// PCB test mode pins
#define LL_PIN__TESTING_MODE_PIN0   24//10//11 rx
#define LL_PIN__TESTING_MODE_PIN1   23// 9//12 tx
#define LL_PULL__TESTING_MODE       LL_GPIO_PULL_UP
#define LL_PIN_Y__TESTING_MODE      0
#define LL_PIN_N__TESTING_MODE      1

#define LL_PIN__PCB_BLUE            3
#define LL_PULLMODE__PCB_BLUE       LL_GPIO_PULL_DOWN
#define LL_PIN_Y__PCB_BLUE          1
#define LL_PIN_N__PCB_BLUE          0

// charging power
#define LL_PIN_CHARGING_POWER       27
#define LL_PULLMODE_CHARGING_POWER  LL_GPIO_PULL_DOWN
#define LL_PIN_Y__CHARGING_POWER    1
#define LL_PIN_N__CHARGING_POWER    0

// ADC: (For nRF52810: P0.02~P0.05 are AIN0~3, P0.28~P0.31 are AIN4~7.)
// battery level
#define LL_ADC_CH_NUM__BATTERY              7 // P.31 is AIN7
#define LL_ADC_CH_NAME__BATTERY             NRF_SAADC_INPUT_AIN7 

// charging 
#define LL_ADC_CH_NUM__CHARGING             4 // P.28 is AIN4
#define LL_ADC_CH_NAME__CHARGING            NRF_SAADC_INPUT_AIN4 

// charging standby
#define LL_ADC_CH_NUM__CHARGING_STANDBY     5 // P.29 is AIN5
#define LL_ADC_CH_NAME__CHARGING_STANDBY    NRF_SAADC_INPUT_AIN5 

// ON/OFF switch of measure circuit:
#define LL_PIN__BATT_LVL         30
#define LL_PULL__BATT_LVL    LL_GPIO_PULL_DOWN//LL_GPIO_PULL_NONE
#define LL_PIN_Y__BATT_LVL  1
#define LL_PIN_N__BATT_LVL  0
// Battery Level: AD * 4 * 1.2 / 1024 = Voltage // 4: hardware design, 1/4 of battery voltage;  1.2: ADC reference voltage;  1024: ADC accurate.
#define AD_VALUE_OF_VOLTAGE(vol_mV) (vol_mV * 1024 / 1200 / 4)

// Charging voltage. 600mV: internal reference voltage  1/6: gain  1024: ADC accurate  
#define VOLTAGE_OF_AD_VALUE(AdValue) (AdValue * 600 * 6 / 1024)
// update the value below by charging/discharging curve of battery:
//
#define POINT_NUM_WHEN_CHARGING   4
#define LL_BATTERY__TABLE_ADC_DESCENDING_WHEN_CHARGING  {AD_VALUE_OF_VOLTAGE(4200), AD_VALUE_OF_VOLTAGE(4190), AD_VALUE_OF_VOLTAGE(3450), AD_VALUE_OF_VOLTAGE(3400) }
#define LL_BATTERY__TABLE_LVL_DESCENDING_WHEN_CHARGING  {100, 91, 2, 0+1} // +1: avoid 0%
#define POINT_NUM_WHEN_DISCHARGING   3
#define LL_BATTERY__TABLE_ADC_DESCENDING_WHEN_DISCHARGING   {AD_VALUE_OF_VOLTAGE(4130), AD_VALUE_OF_VOLTAGE(3350), AD_VALUE_OF_VOLTAGE(3300) }
#define LL_BATTERY__TABLE_LVL_DESCENDING_WHEN_DISCHARGING   {100, 3, 0+1} // +1: avoid 0%
//
// Battery Level Calculate: (refer to "experiment of battery FLP-533433 by Lumen Labs.xlsx")
//#define LL_ADC_VALUE_OF_BATT_LVL_100    AD_VALUE_OF_VOLTAGE(3919)//836 // 836*4*1.2/1024 = 3.919V
//#define LL_ADC_VALUE_OF_BATT_LVL_005    AD_VALUE_OF_VOLTAGE(3352)//715 // 715*4*1.2/1024 = 3.352V
//#define LL_ADC_VALUE_OF_BATT_LVL_000    AD_VALUE_OF_VOLTAGE(3000)//640 // 640*4*1.2/1024 = 3.000V
// Battery Level Indicator when on:
//#define LL_ADC_VALUE_OF_BATT_ALERT  ((LL_ADC_VALUE_OF_BATT_LVL_100 + LL_ADC_VALUE_OF_BATT_LVL_005) / 2)
//#define LL_ADC_VALUE_OF_BATT_OFF    LL_ADC_VALUE_OF_BATT_LVL_005
#define LL_VALUE_OF_BATT_LVL_ALERT  30//53//((LL_ADC_VALUE_OF_BATT_LVL_100 + LL_ADC_VALUE_OF_BATT_LVL_005) / 2)
#define LL_VALUE_OF_BATT_LVL_OFF     5//LL_ADC_VALUE_OF_BATT_LVL_005

#define LL_PIN_CHARGING_5V          4//27 // 5V pin
#define LL_PULLMODE_CHARGING_5V     LL_GPIO_PULL_UP
#define LL_PIN_Y__CHARGING_5V       0
#define LL_PIN_N__CHARGING_5V       1

//LED EN
#define LL_PIN_LED_POWER_ENABLE            5
#define LL_PULLMODE_LED_POWER_ENABLE       LL_GPIO_PULL_DOWN
#define LL_PIN_Y__LED_POWER_ENABLE         1
#define LL_PIN_N__LED_POWER_ENABLE         0

// buttons
#define LL_KEY_PIN          2
#define LL_PIN_Y__KEY       0
#define LL_KEY_NUM_MAX      1
#define LL_KEY_NUM_ONOFF    0
#define LL_KEY_CFG {/* user define number   port    pin      0/1 when released   pull mode           trigger-mode            debounce(ms) */ \
                    {  LL_KEY_NUM_ONOFF,    0,      2/*1*/, 1,                  LL_GPIO_PULL_UP,    LL_GPIO_TRIGGER_LOW,    24            } \
}



// lights
typedef enum
{
    // buzzer
    E_LL_PWM_BUZZER,
    E_LL_LED_FRONT,  
    E_LL_LED_REAR,
    // total number
    E_LL_PWM_CH_NUM,
}E_LL_LED;
typedef enum
{
    LL_LED_GROUP_All = 0,
    LL_LED_GROUP_Side,
    LL_LED_GROUP_Rear,
    LL_LED_GROUP_FrontRear,
    // total number
    LL_LED_GROUP_NUM,
}E_LL_LED_GROUP;

#define LL_PWM_CH_NUM  E_LL_PWM_CH_NUM
#define LL_HW_PWM_PERIOD    370//500//370 // unit:us, 370 is 2.7KHz, 500 is 2KHz, 625 is 1.6KHz, 2315 is 432Hz, 5000 is 200Hz, 10000 is 100Hz
//// power control of LED belt
//#define PIN_LL_LED_BELT_POWER       PIN_OUTPUT_NO_USE//5
//#define LL_LED_BELT_POWER_PULLMODE  LL_GPIO_PULL_NONE
//#define LL_LED_BELT_POWER_NORMAL    0
// see T_LL_PWM_CFG:
#define LL_PWM_CFG { /* user define number              is HW   HW CH   port    pin     0/1 when OFF    pull mode          */  \
                     {  E_LL_PWM_BUZZER,                1,      0,      0,      22,     0,              LL_GPIO_PULL_DOWN   }, \
                     {  E_LL_LED_FRONT,                 2,      0,      0,      26,     0,              LL_GPIO_PULL_DOWN   }, \
                     {  E_LL_LED_REAR,                  2,      1,      0,      25,     0,              LL_GPIO_PULL_DOWN   }, \
}

#define LL_PWM_FREQ_OF_BUZZER   50//100//   // 100 when active, others when passive.

// SPI: G Sensor (from Gavin: "GPIO_to_Other_IC.h" of "LumosAdaptation-Schematic_Ultra ebike_main_V0.0_220214")
#define LL_SPI_PIN_SCK      30  // hardware schematic "G_SCL_SPC"
#define LL_SPI_PIN_MOSI     20  // hardware schematic "G_SDA"
#define LL_SPI_PIN_MISO     28  // hardware schematic "G_SDO_SAO"
#define LL_SPI_PIN_CS       27  // hardware schematic "G_CS"



// flash
#define LL_Flash_Size_of_Para   4096



#endif
