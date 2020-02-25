/**
 * @file mysensors.c
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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "mysensors.h"


/*
 * Buffer sizes in bytes.
 */
#define UART_BUFFER_SIZE      128
#define PAYLOAD_BUFFER_SIZE   32


/*
 * API UART codes.
 */
#define MYSENSORS_NODE_ID_LOCAL  100
#define MYSENSORS_NODE_ID_EXT    103
#define MYSENSORS_NODE_ID_DEBUG  133

#define MYSENSORS_CHILD_ID_TEMP   0
#define MYSENSORS_CHILD_ID_HUM    1
#define MYSENSORS_CHILD_ID_DEBUG  33

#define MYSENSORS_CMD_PRESENTATION   0
#define MYSENSORS_CMD_SET            1

#define MYSENSORS_ACK_NONE  0

#define MYSENSORS_TYPE_PRES_TEMP    6
#define MYSENSORS_TYPE_PRES_HUM     7
#define MYSENSORS_TYPE_PRES_CUSTOM  23
#define MYSENSORS_TYPE_PRES_INFO    36

#define MYSENSORS_TYPE_SET_TEMP    0
#define MYSENSORS_TYPE_SET_HUM     1
#define MYSENSORS_TYPE_SET_VAR1    24
#define MYSENSORS_TYPE_SET_TEXT    47
#define MYSENSORS_TYPE_SET_CUSTOM  48


/**
 * MySensors structure. 
 */ 
typedef struct
{
  int32_t node_id; /*!< node ID (see MYSENSORS_NODE_ID_XXX declarations) */
  int32_t child_sensor_id; /*!< child ID (see MYSENSORS_CHILD_ID_XXX declarations) */
  int32_t command; /*!< command (see MYSENSORS_CMD_XXX declarations) */
  int32_t ack; /*!< acknowledge (see MYSENSORS_ACK_XXX declarations) */
  int32_t type; /*!< payload type (see MYSENSORS_TYPE_XXX declarations) */
  char * payload; /*!< payload */
} MYSENSORS_t;


/* 
 * Variables and buffers.
 */
static UART_HandleTypeDef * mysens_huart;
static uint32_t mysens_uart_buf[UART_BUFFER_SIZE / sizeof(uint32_t)];
static uint32_t mysens_payload_buf[PAYLOAD_BUFFER_SIZE / sizeof(uint32_t)];


/**
 * Sends a MySensors message by UART to the Linux server.
 * 
 * @param sersor pointer to MySensors structure.
 * 
 * @return void.
 */
static void send(MYSENSORS_t * sensor)
{
  /* form message */
  int32_t size = sprintf((char *)&mysens_uart_buf[0], "%d;%d;%d;%d;%d;%s\n",
      (int)sensor->node_id, (int)sensor->child_sensor_id, (int)sensor->command,
      (int)sensor->ack, (int)sensor->type, sensor->payload);

  /* check */
  assert(size < UART_BUFFER_SIZE);

  /* send message */
  HAL_UART_Transmit(mysens_huart, (uint8_t *)mysens_uart_buf, size, 10000);
}

/**
 * Sends a measurement to the Linux server.
 * 
 * @param node node ID.
 * @param child child ID.
 * @param type variable type.
 * @param data_x10 variable multiplied by 10 (to manipulate as integer).
 * 
 * @return void.
 */
static void send_temper_hum(int32_t node, int32_t child, int32_t type, int32_t data_x10)
{
  /* default structure */
  MYSENSORS_t sens = {
      node, child,
      MYSENSORS_CMD_SET, MYSENSORS_ACK_NONE,
      type, (char *)mysens_payload_buf
  };

  /* payload */
  const int32_t size = sprintf(sens.payload, "%d.%d",
      (int)(data_x10 / 10), (int)(data_x10 % 10));

  /* send */
  if (size > 0)
  {
    send(&sens);
  }
}

/**
 * Initializes the module.
 * 
 * @param huart pointer to UART structure.
 * 
 * @return void.
 */
void MYSENSORS_Init(UART_HandleTypeDef * huart)
{
  mysens_huart = huart;
}

/**
 * Sends the local temperature to the Linux server.
 * 
 * @param temper temperature multiplied by 10 (to manipulate as integer).
 * 
 * @return void.
 */
void MYSENSORS_LocalTemperSend(int32_t temper)
{
  send_temper_hum(MYSENSORS_NODE_ID_LOCAL,
      MYSENSORS_CHILD_ID_TEMP,
      MYSENSORS_TYPE_SET_TEMP, temper);
}

/**
 * Sends the local humidity to the Linux server.
 * 
 * @param hum humidity multiplied by 10 (to manipulate as integer).
 * 
 * @return void.
 */
void MYSENSORS_LocalHumiditySend(int32_t hum)
{
  send_temper_hum(MYSENSORS_NODE_ID_LOCAL,
      MYSENSORS_CHILD_ID_HUM,
      MYSENSORS_TYPE_SET_HUM, hum);
}

/**
 * Sends the external temperature to the Linux server.
 * 
 * @param temper temperature multiplied by 10 (to manipulate as integer).
 * 
 * @return void.
 */
void MYSENSORS_ExtTemperSend(int32_t temper)
{
  send_temper_hum(MYSENSORS_NODE_ID_EXT,
      MYSENSORS_CHILD_ID_TEMP,
      MYSENSORS_TYPE_SET_TEMP, temper);
}

/**
 * Sends the external humidity to the Linux server.
 * 
 * @param hum humidity multiplied by 10 (to manipulate as integer).
 * 
 * @return void.
 */
void MYSENSORS_ExtHumiditySend(int32_t hum)
{
  send_temper_hum(MYSENSORS_NODE_ID_EXT,
      MYSENSORS_CHILD_ID_HUM,
      MYSENSORS_TYPE_SET_HUM, hum);
}

/**
 * Sends a debug code to the  Linux server.
 * 
 * @param debug debug code to send.
 * 
 * @return void.
 */
void MYSENSORS_DebugSend(int32_t debug)
{
  /* default structure */
  MYSENSORS_t sens = {
      MYSENSORS_NODE_ID_DEBUG, MYSENSORS_CHILD_ID_DEBUG,
      MYSENSORS_CMD_SET, MYSENSORS_ACK_NONE,
      MYSENSORS_TYPE_SET_TEXT, (char *)mysens_payload_buf
  };

  /* payload */
  const int32_t size = sprintf(sens.payload, "%d", (int)debug);

  /* send */
  if (size > 0)
  {
    send(&sens);
  }
}
