################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../ns-3/src/core/bindings/module_helpers.cc 

OBJS += \
./ns-3/src/core/bindings/module_helpers.o 

CC_DEPS += \
./ns-3/src/core/bindings/module_helpers.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/src/core/bindings/%.o: ../ns-3/src/core/bindings/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


