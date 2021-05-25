/**
 * @file mysensors.c
 * 
 * Data Server STM32 - low level application for the Smart Home data acquisition.
 * Copyright (C) 2020-2021 tuppi-ovh
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

#ifndef MYSENSORS_H
#define MYSENSORS_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

void MYSENSORS_Init(UART_HandleTypeDef * huart);
void MYSENSORS_LocalTemperSend(int32_t temper);
void MYSENSORS_LocalHumiditySend(int32_t hum);
void MYSENSORS_ExtTemperSend(int32_t temper);
void MYSENSORS_ExtHumiditySend(int32_t hum);
void MYSENSORS_DebugSend(int32_t debug);

#endif
