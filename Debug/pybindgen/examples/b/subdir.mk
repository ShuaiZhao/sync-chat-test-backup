################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../pybindgen/examples/b/b.cc 

OBJS += \
./pybindgen/examples/b/b.o 

CC_DEPS += \
./pybindgen/examples/b/b.d 


# Each subdirectory must supply rules for building sources it contributes
pybindgen/examples/b/%.o: ../pybindgen/examples/b/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

