/**
 * @file dht22.h
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

#ifndef DHT22_H_
#define DHT22_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"

void DHT22_Init(TIM_HandleTypeDef * htim_us, GPIO_TypeDef * gpio_port, int32_t gpio_pin);
int32_t DHT22_AnalyseData(uint32_t * buffer, int32_t len, uint32_t * temper, uint32_t * rh);
void DHT22_StartSensor(void);

#endif /* DHT22_H_ */
