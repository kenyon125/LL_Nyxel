/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_INTERFACE_WITHAPP_H
#define _LL_INTERFACE_WITHAPP_H


#define LL_PART_BACK_LIGHT    0
#define LL_PART_FRONT_LIGHT  31 // both front light

#define LL_TYPE_COMMON_ANIMATION 0
#define LL_TYPE_SPECIAL_ANIMATION 1
void packFlashModeParaToApp(unsigned char *buf, unsigned long mode, unsigned long part, unsigned char animation_type, unsigned long state_of_timeslot23to0);
void unpackFlashModeParaFromApp(unsigned char *buf, unsigned long *mode, unsigned long *part, unsigned long *state_of_timeslot23to0);

void packCurrentModeToApp(unsigned char *buf, unsigned long mode);
void unpackCurrentModeFromApp(unsigned char *buf, unsigned long *mode);



#endif
