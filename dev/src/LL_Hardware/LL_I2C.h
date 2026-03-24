/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#ifndef _LL_I2C_H
#define _LL_I2C_H



#include <stdbool.h> // bool

void LL_I2C__Init(void);
void LL_I2C__UnInit(void);

/* @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a transmit buffer.
 * @param[in] length     Number of bytes to send.
 * @param[in] no_stop    If set, the stop condition is not generated on the bus
 *                       after the transfer has completed successfully (allowing
 *                       for a repeated start in the next transfer).
 */
void LL_I2C__Tx(unsigned char address, unsigned char  const *p_data, unsigned char length, bool no_stop);
void LL_I2C__TxByte( unsigned char device_address, unsigned char reg_addr, unsigned char data);
void LL_I2C__TxChunk(unsigned char device_address, unsigned char *p_data, unsigned char length); // reg_addr is in p_data[0]
unsigned long LL_I2C__TxDone(void); // return 0/1: No/Yes

/* @param[in] address    Address of a specific slave device (only 7 LSB).
 * @param[in] p_data     Pointer to a receive buffer.
 * @param[in] length     Number of bytes to be received.
 */
void LL_I2C__Rx(unsigned char device_address, unsigned char *p_data, unsigned char length);
unsigned long LL_I2C__RxDone(void); // return 0/1: No/Yes



#endif
