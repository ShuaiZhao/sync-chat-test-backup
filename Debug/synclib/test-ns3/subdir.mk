################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../synclib/test-ns3/test_ccnx_wrapper.cc \
../synclib/test-ns3/test_digest.cc \
../synclib/test-ns3/test_leaf.cc \
../synclib/test-ns3/test_sync_logic.cc 

OBJS += \
./synclib/test-ns3/test_ccnx_wrapper.o \
./synclib/test-ns3/test_digest.o \
./synclib/test-ns3/test_leaf.o \
./synclib/test-ns3/test_sync_logic.o 

CC_DEPS += \
./synclib/test-ns3/test_ccnx_wrapper.d \
./synclib/test-ns3/test_digest.d \
./synclib/test-ns3/test_leaf.d \
./synclib/test-ns3/test_sync_logic.d 


# Each subdirectory must supply rules for building sources it contributes
synclib/test-ns3/%.o: ../synclib/test-ns3/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


