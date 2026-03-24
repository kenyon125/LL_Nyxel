/**
 * Copyright (c) 2016 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup bootloader_secure_ble main.c
 * @{
 * @ingroup dfu_bootloader_api
 * @brief Bootloader project main file for secure DFU.
 *
 */

#include <stdint.h>
#include "boards.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_bootloader_dfu_timers.h"
#include "nrf_dfu.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "nrf_bootloader_info.h"
#include "nrf_delay.h"

// power button
#define LL_PIN__POWER_BUTTON            2
#define LL_PULL_MODE__POWER_BUTTON      NRF_GPIO_PIN_PULLUP
#define LL_TRIGGER_MODE__POWER_BUTTON   NRF_GPIO_PIN_SENSE_LOW
#define LL_PIN_Y__POWER_BUTTON          0
#define LL_PIN_N__POWER_BUTTON          1

bool is_button_pressed(void) {
    if(LL_PIN_Y__POWER_BUTTON == nrf_gpio_pin_read(LL_PIN__POWER_BUTTON)) {
        return true; 
    } else {
        return false;
    }
}

unsigned long gulResetReason;
bool is_reset_by_button(void) {
    if( 1 == ((gulResetReason & (0x1<<16)) >> 16) ) { // if reset by key (or by reset pin when key pressed)
        #if 0 // can not return true directly, since it maybe due to reset pin when key pressed
            return true; 
        #else
            if(0x1 == (gulResetReason&0x1)) { // if reset by reset pin
                return false;
            } else { // else, must be by key really
                return true; 
            }
        #endif
    } else { // else, this reset not due to key
        return false;
    }
}

// Power off when button pressed.
// Should only work when there is DFU "Idle" state, but Nordic has no such state.
unsigned char ExitBootloaderFlag = 0;
unsigned long debounce_cnt = 0;
static unsigned long prev_button_state = 0xFFFFFFFF; // 0xFFFFFFFF: wait a released state.
void power_off_when_button_pressed(void) {
    unsigned long  now_button_state = nrf_gpio_pin_read(LL_PIN__POWER_BUTTON);
    if(0xFFFFFFFF == prev_button_state) { // wait a released button state, to start this routine
        if(LL_PIN_N__POWER_BUTTON == now_button_state) { // button released
            prev_button_state = LL_PIN_N__POWER_BUTTON;
            debounce_cnt = 0;
        } else { /* keep waiting button released */ }
    } else { // power off when button pressed
        if(prev_button_state != now_button_state) {
            if(debounce_cnt > 3) {
                //LL_PIN_N__BUZZER(LL_PIN__PWM_BUZZER);
                ExitBootloaderFlag = 1;
                sd_power_system_off();
                LL_PIN_N__BUZZER(LL_PIN__PWM_BUZZER);
                while(LL_PIN_Y__POWER_BUTTON == (is_button_pressed() ? LL_PIN_Y__POWER_BUTTON : LL_PIN_N__POWER_BUTTON)); // wait button release.

            } else { /* keep waiting debounce timeout */ }
        } else { debounce_cnt = 0; }
    }
}

static void on_error(void)
{
    NRF_LOG_FINAL_FLUSH();

#if NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
    // To allow the buffer to be flushed by the host.
    nrf_delay_ms(100);
#endif
#ifdef NRF_DFU_DEBUG_VERSION
    NRF_BREAKPOINT_COND;
#endif
    NVIC_SystemReset();
}


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    on_error();
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
    on_error();
}


void app_error_handler_bare(uint32_t error_code)
{
    NRF_LOG_ERROR("Received an error: 0x%08x!", error_code);
    on_error();
}

/**
 * @brief Function notifies certain events in DFU process.
 */
static void dfu_observer(nrf_dfu_evt_type_t evt_type)
{
    switch (evt_type)
    {
        case NRF_DFU_EVT_DFU_FAILED:
        case NRF_DFU_EVT_DFU_ABORTED:
        case NRF_DFU_EVT_DFU_INITIALIZED:
            break;
        case NRF_DFU_EVT_TRANSPORT_ACTIVATED:
            break;
        case NRF_DFU_EVT_DFU_STARTED:
            break;
        default:
            break;
    }
}


/**@brief Function for application main entry. */
int main(void)
{
    uint32_t ret_val;
    
    gulResetReason = NRF_POWER->RESETREAS; NRF_POWER->RESETREAS = gulResetReason;//0xFFFFFFFF; 	
    

    nrf_gpio_cfg_output(LL_PIN__PWM_BUZZER);      LL_PIN_N__BUZZER(LL_PIN__PWM_BUZZER);
    
    // Must happen before flash protection is applied, since it edits a protected page.
    nrf_bootloader_mbr_addrs_populate();

    // Protect MBR and bootloader code from being overwritten.
    ret_val = nrf_bootloader_flash_protect(0, MBR_SIZE);
    APP_ERROR_CHECK(ret_val);
    ret_val = nrf_bootloader_flash_protect(BOOTLOADER_START_ADDR, BOOTLOADER_SIZE);
    APP_ERROR_CHECK(ret_val);

    (void) NRF_LOG_INIT(nrf_bootloader_dfu_timer_counter_get);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("Inside main");

    ret_val = nrf_bootloader_init(dfu_observer);
    APP_ERROR_CHECK(ret_val);

    NRF_LOG_FLUSH();

    NRF_LOG_ERROR("After main, should never be reached.");
    NRF_LOG_FLUSH();

    APP_ERROR_CHECK_BOOL(false);
}

/**
 * @}
 */
