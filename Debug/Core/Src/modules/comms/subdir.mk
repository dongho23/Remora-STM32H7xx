################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/modules/comms/RemoraComms.cpp 

OBJS += \
./Core/Src/modules/comms/RemoraComms.o 

CPP_DEPS += \
./Core/Src/modules/comms/RemoraComms.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/modules/comms/%.o Core/Src/modules/comms/%.su: ../Core/Src/modules/comms/%.cpp Core/Src/modules/comms/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../FATFS/Target -I../FATFS/App -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-modules-2f-comms

clean-Core-2f-Src-2f-modules-2f-comms:
	-$(RM) ./Core/Src/modules/comms/RemoraComms.d ./Core/Src/modules/comms/RemoraComms.o ./Core/Src/modules/comms/RemoraComms.su

.PHONY: clean-Core-2f-Src-2f-modules-2f-comms

