# executables
GCC    := /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/bin/arm-atollic-eabi-gcc
SPLINT := ~/git/splint/build/bin/splint

# flags 
CFLAGS := -Wall -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd
LDFLAGS := -T STM32F100C8_FLASH.ld
SPLINT_FLAGS:=-csv splint.csv +csvoverwrite -line-len 500

# includes 
INCLUDES :=
INCLUDES := $(INCLUDES) -I External/Drivers/STM32F1xx_HAL_Driver/Inc/
INCLUDES := $(INCLUDES) -I External/Drivers/CMSIS/Device/ST/STM32F1xx/Include/
INCLUDES := $(INCLUDES) -I External/Drivers/CMSIS/Include/
INCLUDES := $(INCLUDES) -I Inc/

# defines 
DEFINES := 
DEFINES := $(DEFINES) -D STM32F100xB

SOURCES := 
SOURCES := $(SOURCES) External/Drivers/STM32F1xx_HAL_Driver/Src/*
SOURCES := $(SOURCES) External/Startup/*
SOURCES := $(SOURCES) Src/*


SPLINT_INC:=
SPLINT_INC:=$(SPLINT_INC) -IInc/ 
SPLINT_INC:=$(SPLINT_INC) -IDrivers/STM32F1xx_HAL_Driver/Inc/ 
SPLINT_INC:=$(SPLINT_INC) -IDrivers/CMSIS/Device/ST/STM32F1xx/Include/ 
SPLINT_INC:=$(SPLINT_INC) -IDrivers/CMSIS/Include/
SPLINT_INC:=$(SPLINT_INC) -I../../../99-libs/LaCrosse/
SPLINT_DEF:=-DSTM32F100xB -D__GNUC__


debug: 
	echo $(SOURCES)
	echo $(SOURES)

all: 
	$(GCC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(DEFINES) $(SOURCES)

splint:
	$(SPLINT) $(SPLINT_FLAGS) $(SPLINT_INC) $(SPLINT_DEF) $(SOURCES)
