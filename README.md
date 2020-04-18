# Introduction

Data Server STM32 is a low level application for the Smart Home data acquisition.


# Hardware

Refer to https://tuppi.ovh/data_server_stm32/doc_data_server.


# Compilation

## Compilation Tools 

- Atollic TrueSTUDIO 9.3
- STM32CubeMX


## External Code Sources

To compile the whole project you need to generate the TrueSTUDIO project from STM32CubeMX.

After that you can find some new modules inside your project folder: Drivers, TrueSTUDIO. 

You need to add some code in generated files.

**TrueSTUDIO/startup_stm32f100xb.s:**
- You need to add this line ` ldr sp, =_estack ` before this section:

```
  movs r1, #0
  b LoopCopyDataInit
```

**Src/main.c:**
- You need to insert this code into `main()` function:
```c
  extern void SERV_Init(UART_HandleTypeDef * huart, TIM_HandleTypeDef * htim);
  extern void SERV_Routine(void);
  SERV_Init(&huart1, &htim2);
  while (1)
  {
    SERV_Routine();
  }
``` 

**Src/tiny_printf.c:**
- Rightclick on the project -> New -> Other -> Library functions. And and add "Tiny printf implementation".

**Src/stm32f1xx_it.c:**
- You need to insert this code into `SysTick_Handler()` function:
```c
  extern void SERV_TickIncrement(void);
  SERV_TickIncrement();
```


# LACROSSE TX141TH-BV2 Sensor

Refer to https://tuppi.ovh/data_server_stm32/doc_lacrosse. 


# License 

Refer to the [LICENSE](LICENSE) file.

