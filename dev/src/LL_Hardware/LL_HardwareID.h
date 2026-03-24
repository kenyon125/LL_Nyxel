/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_HARDWAREID_H
#define _LL_HARDWAREID_H



// Vender:
#define LL_HARDWARE_VENDER_NORDIC   ((unsigned char)0x00)
#define LL_HARDWARE_VENDER_TI       ((unsigned char)0x01)

// Hardware ID:
#define LL_HARDWARE_ID_LEN  8
typedef struct
{
    unsigned char Vender;                   // LL_HARDWARE_VENDER_XXX
    unsigned char auc[LL_HARDWARE_ID_LEN];  // [0] is LSB, [7] is MSB.
}T_LL_HardwareID;

void LL_HardwareID_Get(T_LL_HardwareID *ptID);
unsigned long LL_HardwareID_IsEqual(T_LL_HardwareID *ptIDa, T_LL_HardwareID *ptIDb); // return: 0 means not equal, 1 means equal.



#endif
