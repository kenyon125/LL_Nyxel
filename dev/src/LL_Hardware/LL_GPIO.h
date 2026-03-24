/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_GPIO_H
#define _LL_GPIO_H



// LL_GPIO_PULL_xxx:
#define LL_GPIO_PULL_NONE   0
#define LL_GPIO_PULL_UP     1
#define LL_GPIO_PULL_DOWN   2

// LL_GPIO_TRIGGER_xxx:
#define LL_GPIO_TRIGGER_NONE   0
#define LL_GPIO_TRIGGER_LOW    1
#define LL_GPIO_TRIGGER_HIGH   2

/*******************************************************************************
 @brief
 @details
 @param[in] ulPort   : port
 @param[in] ulPin    : pin
 @param[in] ulPull   : LL_GPIO_PULL_xxx
 @param[in] ulTrigger: LL_GPIO_TRIGGER_xxx
 @retval : none
*******************************************************************************/
void LL_GPIO_InputCfg(
                    unsigned long ulPort    ,
                    unsigned long ulPin     ,
                    unsigned long ulPull    ,
                    unsigned long ulTrigger );
                    
/*******************************************************************************
 @brief
 @details
 @param[in] ulPort   : port
 @param[in] ulPin    : pin
 @retval : 0/1
*******************************************************************************/
unsigned long LL_GPIO_InputRead(
                    unsigned long ulPort    ,
                    unsigned long ulPin     );

/*******************************************************************************
 @brief
 @details
 @param[in] ulPort   : port
 @param[in] ulPin    : pin
 @param[in] ulPull   : LL_GPIO_PULL_xxx
 @param[in] init     : 0 means initial low, 1 means initial high.
 @retval : none
*******************************************************************************/
void LL_GPIO_OutputCfg(
                    unsigned long ulPort ,
                    unsigned long ulPin  ,
                    unsigned long ulPull ,
                    unsigned long init   );

/*******************************************************************************
 @brief
 @details
 @param[in] ulPort   : port
 @param[in] ulPin    : pin
 @param[in] value    : 0/1
 @retval : none
*******************************************************************************/
void LL_GPIO_OutputWrite(
                    unsigned long ulPort ,
                    unsigned long ulPin  ,
                    unsigned long value  );



#endif
