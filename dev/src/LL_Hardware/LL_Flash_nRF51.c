/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
//#include "app_uart.h"
#include "app_error.h"
#include "nordic_common.h"
#include "LL_Flash.h"

void LL_Flash_ErasePage(unsigned long * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}


void LL_Flash_WriteWord(unsigned long * address, unsigned long value)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *address = value;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}

#if 0
int main_flash(void)
{
    uint32_t * addr;
    uint8_t    patwr;
    uint8_t    patrd;
    uint8_t    patold;
    uint32_t   i;
    uint32_t   pg_size;
    uint32_t   pg_num;

    uint32_t err_code;
    
    printf("Flashwrite example\n\r");
    patold  = 0;
    pg_size = NRF_FICR->CODEPAGESIZE;
    pg_num  = NRF_FICR->CODESIZE - 1;  // Use last page in flash

    while (true)
    {
        // Start address:
        addr = (uint32_t *)(pg_size * pg_num);
        // Erase page:
        flash_page_erase(addr);
        i = 0;

        do
        {
            printf("Enter char to write to flash\n\r");

            // Read char from uart, and write it to flash:
            do
            {
               //err_code = app_uart_get(&patwr);
            }
            while(err_code == NRF_ERROR_NOT_FOUND);

            if (patold != patwr)
            {
                patold = patwr;
                flash_word_write(addr, (uint32_t)patwr);
                ++addr;
                i += 4;
                printf("'%c' was write to flash\n\r", patwr);
            }
            // Read from flash the last written data and send it back:
            patrd = (uint8_t)*(addr - 1);
            printf("'%c' was read from flash\n\r\n\r", patrd);
        }
        while (i < pg_size);
    }
}
#endif
