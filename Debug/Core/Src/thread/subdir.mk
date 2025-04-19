################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/thread/pruThread.cpp \
../Core/Src/thread/pruTimer.cpp \
../Core/Src/thread/timerInterrupt.cpp 

OBJS += \
./Core/Src/thread/pruThread.o \
./Core/Src/thread/pruTimer.o \
./Core/Src/thread/timerInterrupt.o 

CPP_DEPS += \
./Core/Src/thread/pruThread.d \
./Core/Src/thread/pruTimer.d \
./Core/Src/thread/timerInterrupt.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/thread/%.o Core/Src/thread/%.su Core/Src/thread/%.cyclo: ../Core/Src/thread/%.cpp Core/Src/thread/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++17 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../FATFS/Target -I"F:/Local Repositories/Remora-STM32CubeIDE/Remora-STM32H7xx/Core/Src/lib/ArduinoJson7" -I../FATFS/App -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-thread

clean-Core-2f-Src-2f-thread:
	-$(RM) ./Core/Src/thread/pruThread.cyclo ./Core/Src/thread/pruThread.d ./Core/Src/thread/pruThread.o ./Core/Src/thread/pruThread.su ./Core/Src/thread/pruTimer.cyclo ./Core/Src/thread/pruTimer.d ./Core/Src/thread/pruTimer.o ./Core/Src/thread/pruTimer.su ./Core/Src/thread/timerInterrupt.cyclo ./Core/Src/thread/timerInterrupt.d ./Core/Src/thread/timerInterrupt.o ./Core/Src/thread/timerInterrupt.su

.PHONY: clean-Core-2f-Src-2f-thread

