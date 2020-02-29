/**
 * @file lacrosse.h
 * 
 * @brief This module is shared between two projects: STM32 which is able to 
 * receive 433 MHz only and ARDUINO which is able both to send and receive 
 * 433 MHz.
 * 
 * Data Server STM32 - low level application for the Smart Home data acquisition.
 * Copyright (C) 2020 Vadim MUKHTAROV
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * For information on Data Server STM32: tuppi.ovh@gmail.com
 */

#ifndef LACROSSE_H
#define LACROSSE_H

/* code shared with Arduino 433 MHz transmitter, not debugged yet */
#ifdef STM32F100xB
#define STM32_RX_ONLY_ENABLED
#else 
#define ARDUINO_RX_TX_ENABLED
#endif

/* include arduino header */
#ifdef ARDUINO_RX_TX_ENABLED
#include "Arduino.h"
#endif

/* module version */
#define LACROSSE_VERSION  "0.02"

/* STM32 C functions */ 
#ifdef STM32_RX_ONLY_ENABLED

#ifdef __cplusplus
extern "C"
#endif
uint32_t LACROSSE_input_handler_c(uint32_t duration_usec);

#endif

/* Arduino C++ functions */ 
#ifdef ARDUINO_RX_TX_ENABLED 

void LACROSSE_init(void (*pin_switch)(int32_t), void (*sleep_us)(int32_t));
uint32_t LACROSSE_input_handler(uint32_t duration_usec);
void LACROSSE_output_send(uint32_t sync, uint32_t data);
uint32_t LACROSSE_decrypt_24bits(uint32_t data);
uint32_t LACROSSE_encrypt_24bits(uint32_t data);

#endif

#endif
