################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../synclib/ns3/disabled/sync-ns3-name-info.cc 

OBJS += \
./synclib/ns3/disabled/sync-ns3-name-info.o 

CC_DEPS += \
./synclib/ns3/disabled/sync-ns3-name-info.d 


# Each subdirectory must supply rules for building sources it contributes
synclib/ns3/disabled/%.o: ../synclib/ns3/disabled/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


