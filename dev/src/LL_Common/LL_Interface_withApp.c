/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Interface_withApp.h"

#define MAX_MODE_NUM    2 // mode 0~2
#define BIT_NUM_OF_FIELD_PART   5   // bit[7..5] is "mode", bit[4..0] is "part"
void packFlashModeParaToApp(unsigned char *buf, unsigned long mode, unsigned long part, unsigned char animation_type, unsigned long state_of_timeslot23to0) {
    buf[0] = 'F';
    buf[1] = (mode << BIT_NUM_OF_FIELD_PART) + part;
    buf[2] = (state_of_timeslot23to0 & 0x000000FF) >>  0;
    buf[3] = (state_of_timeslot23to0 & 0x0000FF00) >>  8;
    buf[4] = (state_of_timeslot23to0 & 0x00FF0000) >> 16; 
    buf[5] = animation_type;    
}
void unpackFlashModeParaFromApp(unsigned char *buf, unsigned long *mode, unsigned long *part, unsigned long *state_of_timeslot23to0) {
    *part                    = buf[1] & (0xFF>>(8-BIT_NUM_OF_FIELD_PART));
    *mode                    = buf[1] >> BIT_NUM_OF_FIELD_PART; if(MAX_MODE_NUM < *mode) { *mode = 0; }
    *state_of_timeslot23to0  = (buf[2]<< 0)
                             + (buf[3]<< 8)
                             + (buf[4]<<16);
}

void packCurrentModeToApp(unsigned char *buf, unsigned long mode) {
    buf[0] = 'f';
    buf[1] = '0' + mode;
}
void unpackCurrentModeFromApp(unsigned char *buf, unsigned long *mode) {
    *mode = buf[1] - '0';
}
