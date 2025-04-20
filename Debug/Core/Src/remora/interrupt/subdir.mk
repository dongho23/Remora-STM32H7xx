################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/remora/interrupt/interrupt.cpp 

OBJS += \
./Core/Src/remora/interrupt/interrupt.o 

CPP_DEPS += \
./Core/Src/remora/interrupt/interrupt.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/remora/interrupt/%.o Core/Src/remora/interrupt/%.su Core/Src/remora/interrupt/%.cyclo: ../Core/Src/remora/interrupt/%.cpp Core/Src/remora/interrupt/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++17 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../FATFS/Target -I"F:/Local Repositories/Remora-STM32CubeIDE/Remora-STM32H7xx/Core/Src/lib/ArduinoJson7" -I../FATFS/App -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-remora-2f-interrupt

clean-Core-2f-Src-2f-remora-2f-interrupt:
	-$(RM) ./Core/Src/remora/interrupt/interrupt.cyclo ./Core/Src/remora/interrupt/interrupt.d ./Core/Src/remora/interrupt/interrupt.o ./Core/Src/remora/interrupt/interrupt.su

.PHONY: clean-Core-2f-Src-2f-remora-2f-interrupt

