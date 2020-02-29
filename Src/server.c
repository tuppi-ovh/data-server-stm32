/**
 * @file server.c
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

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "server.h"
#include "mysensors.h"
#include "dht22.h"
#include "lacrosse.h"


/**
 * Enumeration for switch functions.
 */ 
typedef enum {
  SWITCH_OFF = 0,
  SWITCH_ON = 1
} SWITCH_e;



#define HUART_BUFFER_LEN   50


#define PULSE_OLD_SIZE 128
#define PULSE_CIRCULAR_MASK 0x2FF


#define DHT22_SYSTICK_PERIOD (60 * 1000)  /* period of 60 sec */
#define DHT22_PULSE_MASK     63


#define RADIO_PULSE_MASK     511

/*
 * Defines of GPIO pins.
 */ 
#define GPIO_PIN_LED     (GPIO_PIN_12)
#define GPIO_PIN_DHT22   (GPIO_PIN_8)
#define GPIO_PIN_433MHZ  (GPIO_PIN_11)
#define GPIO_PIN_TIMER   (GPIO_PIN_15)




/* pointers */
static UART_HandleTypeDef * serv_huart = NULL;
static TIM_HandleTypeDef * serv_htim = NULL;

/* DHT22 sensor */
static uint32_t dht22_duration_buffer[DHT22_PULSE_MASK + 1];
static uint32_t dht22_duration_buffer_write;
static uint32_t dht22_compare_old;

/* radio */
static uint32_t radio_duration_buffer[RADIO_PULSE_MASK + 1];
static uint32_t radio_duration_buffer_write;
static uint32_t radio_duration_buffer_read;
static uint32_t radio_compare_old;


/**
 * Switches on / off the user LED.
 * 
 * @param sw switch on / off.
 * 
 * @return void.
 */
static void led_switch(SWITCH_e sw)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_LED, (sw == SWITCH_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/**
 * DH22 sensor handler.
 */
static void dht22_routine(void)
{
  static uint32_t dht22_systick_last = 0;
  static uint32_t temper_last = 0;
  static uint32_t rh_last = 0;

  /* current systick */
  const uint32_t systick = HAL_GetTick();

  /* DHT22 read sensor */
  if ((systick - dht22_systick_last) >= DHT22_SYSTICK_PERIOD)
  {
    uint32_t temper;
    uint32_t rh;

    /* read old conversion */
    const int32_t result = \
        DHT22_AnalyseData(dht22_duration_buffer, dht22_duration_buffer_write, &temper, &rh);

    /* send temperature if ok */
    if (result == 0)
    {
      /* send only if new data (for database size) */
      if ((temper_last != temper) || (rh_last != rh))
      {
        /* led on */
        led_switch(SWITCH_ON);

        /* send data to raspberry pi */
        MYSENSORS_LocalTemperSend((int32_t)temper);
        MYSENSORS_LocalHumiditySend((int32_t)rh);

        /* led off */
        led_switch(SWITCH_OFF);

        /* remember last values */
        temper_last = temper;
        rh_last = rh;
      }
    }

    /* reset counter for a new conversion */
    dht22_duration_buffer_write = 0;

    /* start new conversion */
    DHT22_StartSensor();

    /* remember systick */
    dht22_systick_last = systick;
  }
}

/**
 * Lacrosse temperature/humidity sensor handler.
 */
static void lacrosse_handler(uint32_t payload)
{
  static uint32_t payload_last = 0;

  /* Analyze only if new data (for database size) */
  if (payload != payload_last)
  {
    const uint32_t sync = (payload >> 24) & 0xFF;
    const uint32_t temper = (payload >> 8) & 0xFFF;
    const uint32_t hum = payload & 0xFF;
    
    if (sync == 0xAA)
    {
      /* led on */
      led_switch(SWITCH_ON);

      /* send data to raspberry pi */
      MYSENSORS_ExtTemperSend((int32_t)temper - 500);
      MYSENSORS_ExtHumiditySend((int32_t)hum * 10);
      //MYSENSORS_DebugSend((temper << 16) | (hum << 8) | (chk));

      /* led off */
      led_switch(SWITCH_OFF);

      /* remember last value */
      payload_last = payload;
    }
  }
}

/**
 * Handles a 433 MHz pulse duration from a circular buffer.
 * 
 * @return void.
 */
static void lacrosse_routine(void)
{
  if (radio_duration_buffer_read < radio_duration_buffer_write)
  {
    const uint32_t duration = radio_duration_buffer[(radio_duration_buffer_read++) & RADIO_PULSE_MASK];
    const uint32_t value = LACROSSE_input_handler_c(duration);

    /* handle lacrosse data */
    if (value != 0xFFFFFFFF)
    {
      lacrosse_handler(value);
    }
  }
}

/**
 * Timer Capture Callback. Overwrites default callback.
 * 
 * @param htim pointer to HAL Timer structure.
 * 
 * @return void.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef * htim)
{
  /* continue only if correct htim */
  if (htim == serv_htim)
  {
    /* timer of DHT22 sensor */
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      /* read compare register */
      const uint32_t compare =  __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);

      /* value */
      const uint32_t value = (compare >= dht22_compare_old) ?
          (compare - dht22_compare_old) : (0x10000 + compare - dht22_compare_old);

      /* stock capture */
      dht22_duration_buffer[(dht22_duration_buffer_write++) & DHT22_PULSE_MASK] = value;

      /* memorize compare */
      dht22_compare_old = compare;
    }
    /* timer of 433 MHz sensor */
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
      /* read compare register */
      const uint32_t compare =  __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_3);

      /* value */
      const uint32_t value = (compare >= radio_compare_old) ?
          (compare - radio_compare_old) : (0x10000 + compare - radio_compare_old);

      /* stock capture if pulse more than 100 microsec */
      if (value >= 400)
      {
        radio_duration_buffer[(radio_duration_buffer_write++) & RADIO_PULSE_MASK] = value;
      }

      /* memorize compare */
      radio_compare_old = compare;
    }
    else
    {
      /* do nothing */
    }
  }
}

/**
 * Routine called all time in while(1).
 * 
 * @return void.
 */
void SERV_Routine(void)
{
  /* DHT22 routine */
  dht22_routine();

  /* 433 MHz routine */
  lacrosse_routine();
}

/**
 * Module initialisation.
 * 
 * @param huart pointer to HAL UART to send MySensors data.
 * @param htim pointer to HAL timer to measure pulse durations (433 MHz and DHT22).
 * 
 * @return void.  
 */
void SERV_Init(UART_HandleTypeDef * huart, TIM_HandleTypeDef * htim)
{
  /* stock UART struct */
  serv_huart = huart;
  serv_htim = htim;

  /* led switch off */
  led_switch(SWITCH_OFF);

  /* mysensors init */
  MYSENSORS_Init(serv_huart);

  /* systick 100 ms */
  HAL_SetTickFreq(HAL_TICK_FREQ_10HZ);
  /* capture mode for DHT22 */
  HAL_TIM_IC_Start_IT(serv_htim, TIM_CHANNEL_1);
  /* capture mode for 433MHz */
  HAL_TIM_IC_Start_IT(serv_htim, TIM_CHANNEL_3);
  /* start microsec timer */
  HAL_TIM_Base_Start(serv_htim);

  /* DHT22 init */
  dht22_duration_buffer_write = 0;
  dht22_compare_old = 0;
  DHT22_Init(htim, GPIOA, GPIO_PIN_DHT22);

  /* 433 MHz init */
  radio_duration_buffer_write = 0;
  radio_duration_buffer_read = 0;
  radio_compare_old = 0;
  memset(radio_duration_buffer, 0, sizeof(radio_duration_buffer));
}



