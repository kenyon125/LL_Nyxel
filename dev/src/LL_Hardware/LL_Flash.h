/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_FLASH_H
#define _LL_FLASH_H



// NOTE: no busy-checking or write-retry yet!

#if 1 // copy from flash-write example
#include <stdbool.h>
#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "app_uart.h"
#include "app_error.h"
#include "nordic_common.h"
#endif

#define LL_FLASH_ERASE_VALUE_BYTE   0xFF
#define LL_FLASH_ERASE_VALUE_LONG   0x00

void flash_init(void);

extern unsigned long gulFlashStoreNeeded;
extern unsigned long gulFlashStoreCnt;
void flash_store(void);
void flash_load(void);
bool flash_store_finish(void);
bool flash_gc_finish(void);
void flash_store_beforeSleep(void);

#define LL_FLASH_WAITING_TIME_BEFORE_SLEEP  (98 + 338*1024/1000 + 1280)+1000   // 98ms page-erase, 338us 4-byte-write, 1024 4-byte per page, 1280ms top-prior-BLE-adv. (refer to "chapter 9 Flash memory API" of "S132_SDS_v2.0.pdf")
#define LL_FLASH_GC_WAITING_TIME            1000
#endif
