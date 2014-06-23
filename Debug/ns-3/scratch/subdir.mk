################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../ns-3/scratch/scratch-simulator.cc 

OBJS += \
./ns-3/scratch/scratch-simulator.o 

CC_DEPS += \
./ns-3/scratch/scratch-simulator.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/scratch/%.o: ../ns-3/scratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


