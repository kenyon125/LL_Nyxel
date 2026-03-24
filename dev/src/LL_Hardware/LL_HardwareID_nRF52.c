/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "nrf52.h"

#include "LL_HardwareID.h"



void LL_HardwareID_Get(T_LL_HardwareID *ptID)
{
    ptID->Vender = LL_HARDWARE_VENDER_NORDIC;
    // The data alignment in nRF51 implementation is Little Endian. (see nRF51_RM_v3.0 Page14.)
    ptID->auc[0] = NRF_FICR->DEVICEID[0]      ;
    ptID->auc[1] = NRF_FICR->DEVICEID[0] >>  8;
    ptID->auc[2] = NRF_FICR->DEVICEID[0] >> 16;
    ptID->auc[3] = NRF_FICR->DEVICEID[0] >> 24;
    ptID->auc[4] = NRF_FICR->DEVICEID[1]      ;
    ptID->auc[5] = NRF_FICR->DEVICEID[1] >>  8;
    ptID->auc[6] = NRF_FICR->DEVICEID[1] >> 16;
    ptID->auc[7] = NRF_FICR->DEVICEID[1] >> 24;
}

unsigned long LL_HardwareID_IsEqual(T_LL_HardwareID *ptIDa, T_LL_HardwareID *ptIDb)
{
    // compare .Vender
    if(ptIDa->Vender != ptIDb->Vender) return 0;
    // compare .auc[0~7]
    for(int i = 0; i < LL_HARDWARE_ID_LEN; i++) { if( ptIDa->auc[i] != ptIDb->auc[i] ) return 0; }
    // compare finish
    return 1;
}
