################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/remora/modules/debug/debug.cpp 

OBJS += \
./Core/Src/remora/modules/debug/debug.o 

CPP_DEPS += \
./Core/Src/remora/modules/debug/debug.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/remora/modules/debug/%.o Core/Src/remora/modules/debug/%.su Core/Src/remora/modules/debug/%.cyclo: ../Core/Src/remora/modules/debug/%.cpp Core/Src/remora/modules/debug/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++17 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../FATFS/Target -I"F:/Local Repositories/Remora-STM32CubeIDE/Remora-STM32H7xx/Core/Src/lib/ArduinoJson7" -I../FATFS/App -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-remora-2f-modules-2f-debug

clean-Core-2f-Src-2f-remora-2f-modules-2f-debug:
	-$(RM) ./Core/Src/remora/modules/debug/debug.cyclo ./Core/Src/remora/modules/debug/debug.d ./Core/Src/remora/modules/debug/debug.o ./Core/Src/remora/modules/debug/debug.su

.PHONY: clean-Core-2f-Src-2f-remora-2f-modules-2f-debug

