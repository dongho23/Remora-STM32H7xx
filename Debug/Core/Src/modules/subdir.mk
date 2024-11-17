################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/modules/module.cpp \
../Core/Src/modules/moduleInterrupt.cpp 

OBJS += \
./Core/Src/modules/module.o \
./Core/Src/modules/moduleInterrupt.o 

CPP_DEPS += \
./Core/Src/modules/module.d \
./Core/Src/modules/moduleInterrupt.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/modules/%.o Core/Src/modules/%.su Core/Src/modules/%.cyclo: ../Core/Src/modules/%.cpp Core/Src/modules/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../FATFS/Target -I../FATFS/App -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-modules

clean-Core-2f-Src-2f-modules:
	-$(RM) ./Core/Src/modules/module.cyclo ./Core/Src/modules/module.d ./Core/Src/modules/module.o ./Core/Src/modules/module.su ./Core/Src/modules/moduleInterrupt.cyclo ./Core/Src/modules/moduleInterrupt.d ./Core/Src/modules/moduleInterrupt.o ./Core/Src/modules/moduleInterrupt.su

.PHONY: clean-Core-2f-Src-2f-modules

