################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../sync-chat/extensions/custom-cs/content-store-with-node-access.cc 

OBJS += \
./sync-chat/extensions/custom-cs/content-store-with-node-access.o 

CC_DEPS += \
./sync-chat/extensions/custom-cs/content-store-with-node-access.d 


# Each subdirectory must supply rules for building sources it contributes
sync-chat/extensions/custom-cs/%.o: ../sync-chat/extensions/custom-cs/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


