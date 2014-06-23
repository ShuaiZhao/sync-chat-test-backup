################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../sync-chat/results/log/getgop.cc 

OBJS += \
./sync-chat/results/log/getgop.o 

CC_DEPS += \
./sync-chat/results/log/getgop.d 


# Each subdirectory must supply rules for building sources it contributes
sync-chat/results/log/%.o: ../sync-chat/results/log/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


