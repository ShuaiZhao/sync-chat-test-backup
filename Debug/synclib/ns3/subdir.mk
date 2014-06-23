################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../synclib/ns3/sync-ccnx-wrapper.cc \
../synclib/ns3/sync-logic-helper.cc \
../synclib/ns3/sync-scheduler.cc 

OBJS += \
./synclib/ns3/sync-ccnx-wrapper.o \
./synclib/ns3/sync-logic-helper.o \
./synclib/ns3/sync-scheduler.o 

CC_DEPS += \
./synclib/ns3/sync-ccnx-wrapper.d \
./synclib/ns3/sync-logic-helper.d \
./synclib/ns3/sync-scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
synclib/ns3/%.o: ../synclib/ns3/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


