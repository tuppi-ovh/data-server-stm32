/**
 * @file lacrosse.cpp
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


#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "lacrosse.h"


/* number of repeats to send data in LaCrosse-like format */
#define REPEAT_NUMBER 12

/*
 * 2 - 'start bit'
 * 21 - transition from 'start bit' to '1'
 * 10 - transition from '1' to '0'
 * 00 - transition from '0' to '0'
 * 11 - transition from '1' to '1'
 */
#define RADIO_DURATION_2_LOW         1500
#define RADIO_DURATION_2_HIGH        1800
#define RADIO_DURATION_21_LOW        1200
#define RADIO_DURATION_21_HIGH       1400
#define RADIO_DURATION_10_LOW        400
#define RADIO_DURATION_10_HIGH       600
#define RADIO_DURATION_00_LOW        600
#define RADIO_DURATION_00_HIGH       800
#define RADIO_DURATION_11_LOW        600
#define RADIO_DURATION_11_HIGH       800
#define RADIO_DURATION_01_LOW        800
#define RADIO_DURATION_01_HIGH       1000

#define RADIO_DURATION_SCALING_1     100
#define RADIO_DURATION_SCALING_0     100


/**
 * CRC8 table with polynomial 0x31 and init value 0x00. 
 * 
 * @details For more information refer to http://tuppi.ovh/doc_lacrosse.
 */
static uint8_t crc_table[] = {
                         0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
                         0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
                         0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
                         0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
                         0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
                         0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
                         0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
                         0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
                         0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
                         0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
                         0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
                         0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
                         0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
                         0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
                         0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
                         0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
                         0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
                         0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
                         0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
                         0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
                         0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
                         0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
                         0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
                         0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
                         0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
                         0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
                         0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
                         0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
                         0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
                         0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
                         0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
                         0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC 
};

/**
 * Redirection table found from statistic data.
 * 
 * @details For more information refer to http://tuppi.ovh/doc_lacrosse.
 */ 
static uint8_t crc_redir_table[] = {
                        0 	, 49	, 98	, 83	, 196	, 245	, 166	, 151	, 185	, 136	,
                        219	, 234	, 125	, 76	, 31	, 46	, 67	, 114	, 33	, 16	,
                        135	, 182	, 229	, 212	, 250	, 203	, 152	, 169	, 62	, 15	, 
                        92	, 109	, 134	, 183	, 228	, 213	, 66	, 115	, 32	, 17	, 
                        63	, 14	, 93	, 108	, 251	, 202	, 153	, 168	, 197	, 244	, 
                        167	, 150	,  1	, 48	, 99	, 82	, 124	, 77	, 30	, 47	, 
                        184	, 137	, 218	, 235	, 61	, 12	, 95	, 110	, 249	, 200	, 
                        155	, 170	, 132	, 181	, 230	, 215	, 64	, 113	, 34	, 19	, 
                        126	, 79	, 28	, 45	, 186	, 139	, 216	, 233	, 199	, 246	, 
                        165	, 148	,  3	, 50	, 97	, 80	, 187	, 138	, 217	, 232	, 
                        127	, 78	, 29	, 44	,  2	, 51	, 96	, 81	, 198	, 247	, 
                        164	, 149	, 248	, 201	, 154	, 171	, 60	, 13	, 94	, 111	, 
                        65	, 112	, 35	, 18	, 133	, 180	, 231	, 214	, 122	, 75	,
                        24	, 41	, 190	, 143	, 220	, 237	, 195	, 242	, 161	, 144	,
                        7	  , 54	, 101	, 84	, 57	,  8	, 91	, 106	, 253	, 204	, 
                        159	, 174	, 128	, 177	, 226	, 211	, 68	, 117	, 38	, 23	, 
                        252	, 205	, 158	, 175	, 56	,  9	, 90	, 107	, 69	, 116	, 
                        39	, 22	, 129	, 176	, 227	, 210	, 191	, 142	, 221	, 236	, 
                        123	, 74	, 25	, 40	,  6	, 55	, 100	, 85	, 194	, 243	, 
                        160	, 145	, 71	, 118	, 37	, 20	, 131	, 178	, 225	, 208	, 
                        254	, 207	, 156	, 173	, 58	, 11	, 88	, 105	,  4	, 53	, 
                        102	, 87	, 192	, 241	, 162	, 147	, 189	, 140	, 223	, 238	, 
                        121	, 72	, 27	, 42	, 193	, 240	, 163	, 146	,  5	, 52	, 
                        103	, 86	, 120	, 73	, 26	, 43	, 188	, 141	, 222	, 239	, 
                        130	, 179	, 224	, 209	, 70	, 119	, 36	, 21	, 59	, 10	, 
                        89	, 104	, 255	, 206	, 157	, 172	 
};

#ifdef ARDUINO_RX_TX_ENABLED

/**
 * The interest to use a simple encryption table is to not send the data by 433 MHz in clear.
 * 
 * @details used CRC8 DVB_S2 algorithm from http://www.sunshine2k.de/coding/javascript/crc/crc_js.html.
 */
static uint8_t encrypt_table[] = {
    0x00, 0xF7, 0xB9, 0x4E, 0x25, 0xD2, 0x9C, 0x6B, 0x4A, 0xBD, 0xF3, 0x04, 0x6F, 0x98, 0xD6, 0x21,
    0x94, 0x63, 0x2D, 0xDA, 0xB1, 0x46, 0x08, 0xFF, 0xDE, 0x29, 0x67, 0x90, 0xFB, 0x0C, 0x42, 0xB5,
    0x7F, 0x88, 0xC6, 0x31, 0x5A, 0xAD, 0xE3, 0x14, 0x35, 0xC2, 0x8C, 0x7B, 0x10, 0xE7, 0xA9, 0x5E,
    0xEB, 0x1C, 0x52, 0xA5, 0xCE, 0x39, 0x77, 0x80, 0xA1, 0x56, 0x18, 0xEF, 0x84, 0x73, 0x3D, 0xCA,
    0xFE, 0x09, 0x47, 0xB0, 0xDB, 0x2C, 0x62, 0x95, 0xB4, 0x43, 0x0D, 0xFA, 0x91, 0x66, 0x28, 0xDF,
    0x6A, 0x9D, 0xD3, 0x24, 0x4F, 0xB8, 0xF6, 0x01, 0x20, 0xD7, 0x99, 0x6E, 0x05, 0xF2, 0xBC, 0x4B,
    0x81, 0x76, 0x38, 0xCF, 0xA4, 0x53, 0x1D, 0xEA, 0xCB, 0x3C, 0x72, 0x85, 0xEE, 0x19, 0x57, 0xA0,
    0x15, 0xE2, 0xAC, 0x5B, 0x30, 0xC7, 0x89, 0x7E, 0x5F, 0xA8, 0xE6, 0x11, 0x7A, 0x8D, 0xC3, 0x34,
    0xAB, 0x5C, 0x12, 0xE5, 0x8E, 0x79, 0x37, 0xC0, 0xE1, 0x16, 0x58, 0xAF, 0xC4, 0x33, 0x7D, 0x8A,
    0x3F, 0xC8, 0x86, 0x71, 0x1A, 0xED, 0xA3, 0x54, 0x75, 0x82, 0xCC, 0x3B, 0x50, 0xA7, 0xE9, 0x1E,
    0xD4, 0x23, 0x6D, 0x9A, 0xF1, 0x06, 0x48, 0xBF, 0x9E, 0x69, 0x27, 0xD0, 0xBB, 0x4C, 0x02, 0xF5,
    0x40, 0xB7, 0xF9, 0x0E, 0x65, 0x92, 0xDC, 0x2B, 0x0A, 0xFD, 0xB3, 0x44, 0x2F, 0xD8, 0x96, 0x61,
    0x55, 0xA2, 0xEC, 0x1B, 0x70, 0x87, 0xC9, 0x3E, 0x1F, 0xE8, 0xA6, 0x51, 0x3A, 0xCD, 0x83, 0x74,
    0xC1, 0x36, 0x78, 0x8F, 0xE4, 0x13, 0x5D, 0xAA, 0x8B, 0x7C, 0x32, 0xC5, 0xAE, 0x59, 0x17, 0xE0,
    0x2A, 0xDD, 0x93, 0x64, 0x0F, 0xF8, 0xB6, 0x41, 0x60, 0x97, 0xD9, 0x2E, 0x45, 0xB2, 0xFC, 0x0B,
    0xBE, 0x49, 0x07, 0xF0, 0x9B, 0x6C, 0x22, 0xD5, 0xF4, 0x03, 0x4D, 0xBA, 0xD1, 0x26, 0x68, 0x9F
};

static uint8_t decrypt_table[256];

static void (*p_pin_switch)(int32_t) = NULL;
static void (*p_sleep_us)(int32_t) = NULL;

#endif


/**
 * Calculates a checksum for the 32-bit payload.
 * 
 * @param payload payload.
 * 
 * @return calculated checksum.
 */
static uint32_t checksum_calculate(uint32_t payload)
{
  int32_t i;
  uint32_t crc8_redir;
  uint32_t crc8;

  crc8 = 0u;
  /* loop on 4 bytes */
  for (i = 0; i < 4; i++)
  {
    const uint32_t byte = (payload >> (8 * (3 - i))) & 0xFF;
    crc8 = crc_table[(crc8 & 0xFF) ^ (byte & 0xFF)] & 0xFF;
  }
  crc8_redir = crc_redir_table[crc8];

  return crc8_redir;
}

/**
 * Checksum verification regarding the Lacrosse algorithm.
 * 
 * @details For more information refer to http://tuppi.ovh/doc_lacrosse.
 * 
 * @param payload received payload.
 * @param chk received checksum.
 * 
 * @return error code (0 when no error).
 */
static int32_t checksum_verify(uint32_t payload, uint32_t chk)
{
  return (chk == checksum_calculate(payload)) ? 0 : -1;
}

/**
 * Tries to receive 43 pulses (3 start bits + 32 bits of payload + 8 bits of checksum).
 * 
 * @param duration_usec pulse duration un microsec.
 * 
 * @return 32-bit payload if ok otherwise 0xFFFFFFFF.
 */
uint32_t LACROSSE_input_handler(uint32_t duration_usec)
{
  static int32_t radio_state = 0;
  static uint64_t radio_register = 0;
  static int32_t bit = 0;

  uint32_t retval = 0xFFFFFFFFu;
  const uint32_t duration = duration_usec;

  switch(radio_state)
  {
  /* start bits */
  case 0:
  case 1:
  case 2:
    if ((duration > RADIO_DURATION_2_LOW) && (duration < RADIO_DURATION_2_HIGH))
    {
      radio_state++;
    }
    else
    {
      radio_state = 0;
    }
    break;

  /* first bit */
  case 3:
    if ((duration > RADIO_DURATION_21_LOW) && (duration < RADIO_DURATION_21_HIGH))
    {
      radio_state++;
      bit = 1;
    }
    else
    {
      radio_state = 0;
      bit = 0;
    }
    radio_register = bit;
    break;

  /* 40 bits ready */
  case 43:
    radio_state = 0;
    if (checksum_verify(radio_register >> 8, radio_register & 0xFF) == 0)
    {
      retval = radio_register >> 8;
    }
    break;

  /* from 2nd to 40th bits */
  default:
    if ((duration > RADIO_DURATION_10_LOW) && (duration < RADIO_DURATION_10_HIGH) && (bit == 1))
    {
      radio_state++;
      bit = 0;
    }
    else if ((duration > RADIO_DURATION_00_LOW) && (duration < RADIO_DURATION_00_HIGH) && (bit == 0))
    {
      radio_state++;
      bit = 0;
    }
    else if ((duration > RADIO_DURATION_11_LOW) && (duration < RADIO_DURATION_11_HIGH) && (bit == 1))
    {
      radio_state++;
      bit = 1;
    }
    else if ((duration > RADIO_DURATION_01_LOW) && (duration < RADIO_DURATION_01_HIGH) && (bit == 0))
    {
      radio_state++;
      bit = 1;
    }
    else
    {
      radio_state = 0;
      bit = 0;
    }
    radio_register = (radio_register << 1) | (bit);
    break;
  }

  return retval;
}


/*********************************************************************************/

#ifdef STM32_RX_ONLY_ENABLED

/**
 * Export C of the @ref LACROSSE_input_handler() function.
 */
extern "C" uint32_t LACROSSE_input_handler_c(uint32_t duration_usec)
{
  return LACROSSE_input_handler(duration_usec);
}

#endif


/*********************************************************************************/

#ifdef ARDUINO_TRANSMITTER_ENABLED


/**
 * Sends a required number of bits.
 */
static void send(uint32_t data, int32_t bits)
{
  int32_t i;

  /* durations */
  const uint32_t duration_short = (RADIO_DURATION_10_LOW + RADIO_DURATION_10_HIGH) >> 1;
  const uint32_t duration_long  = (RADIO_DURATION_01_LOW + RADIO_DURATION_01_HIGH) >> 1;

  /* preconditions check */
  assert(p_pin_switch != NULL);
  assert(p_sleep_us != NULL);

  for (i = bits - 1; i >= 0; i--)
  {
    if ((data >> i) & 0x1)
    {
      p_pin_switch(1);
      p_sleep_us(duration_long);
      p_pin_switch(0);
      p_sleep_us(duration_short);
    }
    else
    {
      p_pin_switch(1);
      p_sleep_us(duration_short);
      p_pin_switch(0);
      p_sleep_us(duration_long);
    }
  }
}

/**
 * Module initialization.
 * 
 * @param pin_switch function pointer to activate/deactivate 433 MHz.
 * @param sleep_us function pointer to sleep.
 * 
 * @return void.
 */
void LACROSSE_init(void (*pin_switch)(int32_t), void (*sleep_us)(int32_t))
{
  int32_t i;

  /* pointers */
  p_pin_switch = pin_switch;
  p_sleep_us = sleep_us;

  /* form decrypt table */
  for (i = 0; i < 256; i++)
  {
    decrypt_table[encrypt_table[i]] = 0; /* TODO */
  }
}

/**
 * Sends N packets regarding a LaCrosse protocol.
 */ 
void LACROSSE_output_send(uint32_t sync, uint32_t data)
{
  const uint32_t data_encrypted = LACROSSE_encrypt_24bits(data);
  const uint32_t payload = (sync << 24) | data_encrypted;
  const uint32_t checksum = checksum_calculate(payload);

  const uint32_t duration_start = (RADIO_DURATION_2_LOW + RADIO_DURATION_2_HIGH) >> 2;
  
  int32_t i, k;
  
  /* preconditions check */
  assert(p_pin_switch != NULL);
  assert(p_sleep_us != NULL);
  
  /* scaling */
  p_pin_switch(1);
  p_sleep_us(RADIO_DURATION_SCALING_1);
  p_pin_switch(0);
  p_sleep_us(RADIO_DURATION_SCALING_0);
  
  /* repeat for N times */
  for (k = 0; k < REPEAT_NUMBER; k++)
  {
    /* start bits */
    for (i = 0; i < 4; i++)
    {
      p_pin_switch(1);
      p_sleep_us(duration_start);
      p_pin_switch(0);
      p_sleep_us(duration_start);
    }

    /* payload */
    send(payload, 32);
    
    /* checksum */
    send(checksum, 8);
  }
}

/**
 * Decrypts 24-bit data.
 */
uint32_t LACROSSE_decrypt_24bits(uint32_t data)
{
  const uint32_t byte_0 = (data >> 16) & 0xFF;
  const uint32_t byte_2 = (data >> 8) & 0xFF;
  const uint32_t byte_1 = (data >> 0) & 0xFF;

  return (decrypt_table[byte_0] << 16) | (decrypt_table[byte_2] << 8) | (decrypt_table[byte_1] << 0);
}

/**
 * Encrypts 24-bit data.
 *
 * byte2 byte1 byte0 >> byte0 byte2 byte1 >> encrypt_table.
 */
uint32_t LACROSSE_encrypt_24bits(uint32_t data)
{
  const uint32_t byte_2 = (data >> 16) & 0xFF;
  const uint32_t byte_1 = (data >> 8) & 0xFF;
  const uint32_t byte_0 = (data >> 0) & 0xFF;

  return (encrypt_table[byte_0] << 16) | (encrypt_table[byte_2] << 8) | (encrypt_table[byte_1] << 0);
}

#endif
