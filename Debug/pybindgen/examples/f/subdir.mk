################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../pybindgen/examples/f/f.cc 

OBJS += \
./pybindgen/examples/f/f.o 

CC_DEPS += \
./pybindgen/examples/f/f.d 


# Each subdirectory must supply rules for building sources it contributes
pybindgen/examples/f/%.o: ../pybindgen/examples/f/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


