/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_BLE_H
#define _LL_BLE_H



#define LL_BLE_COMPANY_ID_OF_LUMOS  0x0F04 // READ ONLY!
#define LL_BLE_COMPANY_ID_OF_LUMOS__BYTE_L  ((LL_BLE_COMPANY_ID_OF_LUMOS & 0x00FF) >> 0)
#define LL_BLE_COMPANY_ID_OF_LUMOS__BYTE_H  ((LL_BLE_COMPANY_ID_OF_LUMOS & 0xFF00) >> 8)

//#define LL_BLE_TX_POWER         4 //BLE tx power: -30, -20, -16, -12, -8, -4, 0, and 4 dBm.
#define LL_BLE_ADV_NAME         "Nyxel"
//#define LL_BLE_MANU_NAME        "LumenLabs"
//#define LL_BLE_ADV_INTERVAL     500 // ms, 20ms ~ 10.24s

void on_adv_evt(ble_adv_evt_t ble_adv_evt);
void LL_BLE_Adv_init(ble_advertising_init_t const * const p_init);
void LL_BLE_Adv_start(void);
void LL_BLE_Adv_stop(void);



#endif
