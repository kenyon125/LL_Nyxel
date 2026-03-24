/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_ADC_H
#define _LL_ADC_H



// ADC summary: 8ch(0~7), 10-bit, 1.2V ref.

// sample:
#define LL_ADC_SAMPLE_VALUE_NONE    0xFFFFFFFF
extern volatile unsigned long LL_ADC_Sample;    // sample value, when power on, should be LL_ADC_SAMPLE_VALUE_NONE.
extern volatile unsigned long LL_ADC_SampleSeq; // sample sequence, when power on, should be 0, means not sample yet.
extern volatile signed long LL_ADC_Sample_1;    // sample value(charging), when power on, should be LL_ADC_SAMPLE_VALUE_NONE.
extern volatile signed long LL_ADC_Sample_2;    // sample value(charging standby), when power on, should be LL_ADC_SAMPLE_VALUE_NONE.

/*******************************************************************************
 @brief        : ADC initialization.
 @details      : none
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       : none
*******************************************************************************/
void LL_ADC_Init(void);

/*******************************************************************************
 @brief        : start a conversion.
 @details      : just start, do not mean finished when returns.
 @param[in    ]: none
 @param[   out]: none
 @param[in,out]: none
 @retval       : none
*******************************************************************************/
void LL_ADC_StartSingleConversion(void);//(unsigned long ulCH);



#endif
