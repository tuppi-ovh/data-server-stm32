## Introduction

TBD.

## Compilation Tools 

- Atollic TrueSTUDIO 9.3
- STM32CubeMX

## External Code Sources

To compile the whole project you need to download some modules: 

**External/Drivers/CMSIS:** 
- You can use the CubeMX to configure these files for the Atollic project.

**External/Drivers/STM32F1xx_HAL_Driver:** 
- You can use the CubeMX to configure these files for the Atollic project.

**External/Startup/startup_stm32f100xb.s:**
- You can use the CubeMX to configure these files for the Atollic project.

**External/Main/main.c:**
- You can use the CubeMX to configure these files for the Atollic project.
- You need to insert this code into ̀ main()` function:
```c
  SERV_Init(&huart1, &htim2);
  while (1)
  {
    SERV_Routine();
  }
``` 

# License 

Refer to the LICENSE file.

# Links 

- [DHT22 with STM32](https://www.controllerstech.com/temperature-measurement-using-dht22-in-stm32)
- [MySensors API UART](https://www.mysensors.org/download/serial_api_20)