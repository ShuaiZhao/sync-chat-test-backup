################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../ns-3/src/virtual-net-device/examples/virtual-net-device.cc 

OBJS += \
./ns-3/src/virtual-net-device/examples/virtual-net-device.o 

CC_DEPS += \
./ns-3/src/virtual-net-device/examples/virtual-net-device.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/src/virtual-net-device/examples/%.o: ../ns-3/src/virtual-net-device/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


