/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_KEY_H
#define _LL_KEY_H



#include "LL_Hardware_cfg.h"

// NOTE: Pls define your N of every key, it should be [0, LL_KEY_NUM_MAX).

typedef struct
{
    unsigned long N;        // user define number
    unsigned long port;     // GPIO port
    unsigned long pin;      // GPIO pin
    unsigned long normal;   // 0/1 when released.
    unsigned long pull;     // see LL_GPIO.h LL_GPIO_PULL_xxx
    unsigned long trigger;  // see LL_GPIO.h LL_GPIO_TRIGGER_xxx
    unsigned long debounce; // unit: ms
}T_LL_KEY_CFG;
extern T_LL_KEY_CFG gatKeyCfg[LL_KEY_NUM_MAX];

void LL_Key_Init(void); //(unsigned long N)    
void LL_Key_Scan(void); //(unsigned long N)
void LL_Key_Init_With_No_Trigger(void); //(unsigned long N)

typedef struct
{
    unsigned long N;        // T_LL_KEY_CFG.N
    unsigned long evt;      // 0 means released, 1 means pressed.
}T_LL_KEY_EVT;
unsigned long LL_Key_EvtFetch(T_LL_KEY_EVT *ptEvt); // return 0 means no event, 1 means some event.
unsigned long LL_Key_EvtPeek( T_LL_KEY_EVT *ptEvt); // return 0 means no event, 1 means some event.
void LL_Key_IgnoreNextRelease(unsigned long N);     // After a long press, the next release may be usless.


#endif
