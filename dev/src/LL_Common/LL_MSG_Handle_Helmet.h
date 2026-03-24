/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_MSG_HANDLE_HELMET_H
#define _LL_MSG_HANDLE_HELMET_H



#include "LL_MSG.h"

void LL_HelmetActionWhenStateChanged(void);
extern unsigned long gulMsgHandleNeeded;
extern unsigned long    gulRSSI;
extern T_MSG_toHelmet   sgtMsg;
void LL_MsgHandle_Helmet(T_MSG_toHelmet *ptMsg);

extern unsigned long canThisRemoteChangeSleepTime;
#define LL_DEFAULT_OPTION_SLEEPTIME_OF_REMOTE   1 // 1: add 1 "10s" to the base 20s, so it's 30s
extern unsigned long option_SleepTime_of_remote_from_app;

void LL_MSG_Handle__PublicMessage_saveWhenInInterrupt(unsigned char *pMessage, unsigned long length);
void LL_MSG_Handle__PublicMessage_parseWhenInMainloop(void);

    

#endif
