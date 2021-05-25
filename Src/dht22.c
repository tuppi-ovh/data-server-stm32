/**
 * @file dht22.c
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

#include <stdint.h>
#include <assert.h>

#include "dht22.h"
#include "stm32f1xx_hal.h"

/* DHT22 pulse properties */ 
#define HIGH_MIN 110
#define HIGH_MAX 140
#define LOW_MIN  70
#define LOW_MAX  100

/* local variable declarations */
static TIM_HandleTypeDef * loc_htim_us = NULL;
static int32_t loc_gpio_pin = -1;
static GPIO_TypeDef * loc_gpio_port = NULL;


/**
 * Local delay function (very approximative) based on the HAL timer.
 * 
 * @param delay delay in microsec.
 * 
 * @return void.
 */
static void delay_us(int32_t delay)
{
  uint32_t old = 0xFFFFFFFF;

  /* preconditions check */
  assert(loc_htim_us != NULL);

  /* loop while delay is not gone */
  while(delay > 0)
  {
    if (loc_htim_us->Instance->CNT != old)
    {
      delay--;
      old = loc_htim_us->Instance->CNT;
    }
  }
}

/**
 * Configures the GPIO PA1 as output.
 * 
 * @return void.
 */
static void set_gpio_output(void)
{
  /* preconditions check */
  assert(loc_gpio_pin != -1);
  assert(loc_gpio_port != NULL);

  /* Configure GPIO pin output: PA1 */
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = loc_gpio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(loc_gpio_port, &GPIO_InitStruct);
}

/**
 * Configures the GPIO PA1 as input.
 * 
 * @return void.
 */
static void set_gpio_input(void)
{
  /* preconditions check */
  assert(loc_gpio_pin != -1);
  assert(loc_gpio_port != NULL);

  /* Configure GPIO pin input: PA1 */
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = loc_gpio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(loc_gpio_port, &GPIO_InitStruct);
}

/**
 * Initializes the DHT22 module.
 * 
 * @param htim_us pointer to the timer structure.
 * @param gpio_pin pin index.
 * 
 * @return void.
 */
void DHT22_Init(TIM_HandleTypeDef * htim_us, GPIO_TypeDef * gpio_port, int32_t gpio_pin)
{
  /* init */
  loc_htim_us = htim_us;
  loc_gpio_pin = gpio_pin;
  loc_gpio_port = gpio_port;

  /* default gpio state */
  set_gpio_input();
}

/**
 * Starts the DHT22 sensor acquisition.
 * 
 * @return void.
 */
void DHT22_StartSensor(void)
{
  /* preconditions check */
  assert(loc_gpio_pin != -1);
  assert(loc_gpio_port != NULL);

  /* set the pin as output */
  set_gpio_output();

  /* pull the pin low */
  HAL_GPIO_WritePin(loc_gpio_port, loc_gpio_pin, 0);
  delay_us(500);

  /* pull the pin high */
  HAL_GPIO_WritePin(loc_gpio_port, loc_gpio_pin, 1);
  delay_us(30);

  /* set as input */
  set_gpio_input();
}


/**
 * Analazes the DHT22 response regarding the pulse durations.
 * 
 * @param buffer buffer where pulse durations are written.
 * @param len length of the buffer in 32-bit words.
 * @param temper output temperature. 
 * @param rh output relative humidity.
 * 
 * @return 0 when no error.
 */
int32_t DHT22_AnalyseData(uint32_t * buffer, int32_t len, uint32_t * temper, uint32_t * rh)
{
  int32_t retval = 0;
  uint32_t humidity;
  uint32_t temperature;
  uint32_t sum;

  /* we need exaclty 43 pulses */
  if (len != 43)
  {
    retval = -1;
  }

  /* read RH */
  if (retval == 0)
  {
    int32_t i;
    humidity = 0;
    for (i = 0; i < 16; i++)
    {
      const int32_t duration = buffer[3 + i];
      if ((duration > HIGH_MIN) && (duration < HIGH_MAX))    humidity = (humidity << 1) | 1;
      else if ((duration > LOW_MIN) && (duration < LOW_MAX)) humidity = (humidity << 1) | 0;
      else retval = -2;
    }
  }

  /* read temper */
  if (retval == 0)
  {
    int32_t i;
    temperature = 0;
    for (i = 0; i < 16; i++)
    {
      const int32_t duration = buffer[3 + 16 + i];
      if ((duration > HIGH_MIN) && (duration < HIGH_MAX))    temperature = (temperature << 1) | 1;
      else if ((duration > LOW_MIN) && (duration < LOW_MAX)) temperature = (temperature << 1) | 0;
      else retval = -3;
    }
  }

  /* read summ */
  if (retval == 0)
  {
    int32_t i;
    sum = 0;
    for (i = 0; i < 8; i++)
    {
      const int32_t duration = buffer[3 + 32 + i];
      if ((duration > HIGH_MIN) && (duration < HIGH_MAX))    sum = (sum << 1) | 1;
      else if ((duration > LOW_MIN) && (duration < LOW_MAX)) sum = (sum << 1) | 0;
      else retval = -4;
    }
  }

  /* check sum */
  if (retval == 0)
  {
    uint32_t sum_calc = 0;
    sum_calc += (humidity >> 0) & 0xFF;
    sum_calc += (humidity >> 8) & 0xFF;
    sum_calc += (temperature >> 0) & 0xFF;
    sum_calc += (temperature >> 8) & 0xFF;
    sum_calc &= 0xFF;
    if (sum_calc != sum) retval = -5;
  }

  /* return values */
  if (retval == 0)
  {
    *temper = temperature;
    *rh = humidity;
  }

  return retval;
}



