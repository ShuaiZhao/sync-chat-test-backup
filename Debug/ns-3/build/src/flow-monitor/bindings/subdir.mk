################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../ns-3/build/src/flow-monitor/bindings/ns3module.cc.7.o 

CC_SRCS += \
../ns-3/build/src/flow-monitor/bindings/ns3module.cc 

OBJS += \
./ns-3/build/src/flow-monitor/bindings/ns3module.o 

CC_DEPS += \
./ns-3/build/src/flow-monitor/bindings/ns3module.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/build/src/flow-monitor/bindings/%.o: ../ns-3/build/src/flow-monitor/bindings/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

