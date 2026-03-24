/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

// 2.4GHz module based on Nordic ESB
// TO DO: need a better interface of "change ESB address"

#ifndef _LL_ESB_H
#define _LL_ESB_H



// 0. Dependency
#include <stdint.h>     // uint32_t
#include "nrf_esb.h"    // Nordic ESB
#include "LL_MSG.h"     // T_MSG_toHelmet, should be removed !!!!!!



// 1. Read-Only Info
//
#define LL_ESB_BASE_ADDR_LEN    4



// 2. Functions (basic of a module)
//
void LL_ESB_Init(void); // Call it when initialization.
//
// need these frequent start/stop instead of loop, since it bases on the Nordic Timeslot 
////void LL_ESB_Loop ( void);
void     LL_ESB_Start( void); // Call it when Nordic Timeslot notifies that "BLE released the radio, you can use it now".
uint32_t LL_ESB_Stop(  void); // Call it when Nordic Timeslot notifies that "BLE wants to use the radio now, you must release it at once".



// 3. Functions for sending/receiving data
//
// 1) Pls implement this callback:
void LL_ESB_callback_esb_started(void); // pls implement this
//
// 2) Choose a PTX/PRX role
//      selective_auto_ack:
//          when PRX, set it to false which means "fixed auto ack", so that can always ack-with-payload, compatible with the different radio "noack" bit between Remote Lite and Pro. (same as the setting before our Sync Feature)
//          when PTX, either true/false should be ok, set it to true now.
uint32_t esb_start_PTX(bool selective_auto_ack); // need a return value due to "VERIFY_SUCCESS" did
uint32_t esb_start_PRX(bool selective_auto_ack); // need a return value due to "VERIFY_SUCCESS" did
//
// 3) Write data to sending buffer (even work as a "Ack-Payload" when PRX mode)
void LL_ESB_WriteTxBuf(unsigned char *pucData, unsigned long ulLen, unsigned long pipe, bool noack);
//
// 4) Receive data:
extern unsigned char LL_ESB_data_received; // Indicate whether there is data received, if 1: pls call "nrf_esb_read_rx_payload" to get data and remember to clear it at last.

uint32_t LL_ESB__set_address(void);

//// for LumOS
// TO DO: need a better interface of "change ESB address"
extern unsigned char ESB_address_type; // 0: normal; 1: pairing



#endif
