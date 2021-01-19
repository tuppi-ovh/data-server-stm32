#ifndef stm32f1xx_hal_h
#define stm32f1xx_hal_h

typedef struct 
{
  unsigned int CNT;
} InstanceTypeDef;

typedef struct
{
  InstanceTypeDef Instance;
} TIM_HandleTypeDef;

typedef struct 
{
  unsigned int temp;
} GPIO_TypeDef;

typedef struct 
{
  unsigned int Pin;
  unsigned int Mode;
  unsigned int Speed;
} GPIO_InitTypeDef;

typedef struct 
{
  unsigned int temp;
} UART_HandleTypeDef;


#endif