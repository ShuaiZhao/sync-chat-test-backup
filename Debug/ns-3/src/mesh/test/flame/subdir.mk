################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../ns-3/src/mesh/test/flame/flame-regression.cc \
../ns-3/src/mesh/test/flame/flame-test-suite.cc \
../ns-3/src/mesh/test/flame/regression.cc 

OBJS += \
./ns-3/src/mesh/test/flame/flame-regression.o \
./ns-3/src/mesh/test/flame/flame-test-suite.o \
./ns-3/src/mesh/test/flame/regression.o 

CC_DEPS += \
./ns-3/src/mesh/test/flame/flame-regression.d \
./ns-3/src/mesh/test/flame/flame-test-suite.d \
./ns-3/src/mesh/test/flame/regression.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/src/mesh/test/flame/%.o: ../ns-3/src/mesh/test/flame/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


