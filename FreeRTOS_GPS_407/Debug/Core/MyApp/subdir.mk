################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/MyApp/output.c \
../Core/MyApp/studentstuff.c \
../Core/MyApp/taskcreate.c 

OBJS += \
./Core/MyApp/output.o \
./Core/MyApp/studentstuff.o \
./Core/MyApp/taskcreate.o 

C_DEPS += \
./Core/MyApp/output.d \
./Core/MyApp/studentstuff.d \
./Core/MyApp/taskcreate.d 


# Each subdirectory must supply rules for building sources it contributes
Core/MyApp/%.o: ../Core/MyApp/%.c Core/MyApp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-MyApp

clean-Core-2f-MyApp:
	-$(RM) ./Core/MyApp/output.d ./Core/MyApp/output.o ./Core/MyApp/studentstuff.d ./Core/MyApp/studentstuff.o ./Core/MyApp/taskcreate.d ./Core/MyApp/taskcreate.o

.PHONY: clean-Core-2f-MyApp

