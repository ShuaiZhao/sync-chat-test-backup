################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ns-3/build/.conf_check_ace1886511f08627ee2e4ca020806295/test.cpp 

OBJS += \
./ns-3/build/.conf_check_ace1886511f08627ee2e4ca020806295/test.o 

CPP_DEPS += \
./ns-3/build/.conf_check_ace1886511f08627ee2e4ca020806295/test.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/build/.conf_check_ace1886511f08627ee2e4ca020806295/%.o: ../ns-3/build/.conf_check_ace1886511f08627ee2e4ca020806295/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


