################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../synclib/test/test_app_socket.cc \
../synclib/test/test_ccnx_wrapper.cc \
../synclib/test/test_data_fetch_and_publish.cc \
../synclib/test/test_digest.cc \
../synclib/test/test_interest_table.cc \
../synclib/test/test_leaf.cc \
../synclib/test/test_pit.cc \
../synclib/test/test_scheduler.cc \
../synclib/test/test_sync_logic.cc 

OBJS += \
./synclib/test/test_app_socket.o \
./synclib/test/test_ccnx_wrapper.o \
./synclib/test/test_data_fetch_and_publish.o \
./synclib/test/test_digest.o \
./synclib/test/test_interest_table.o \
./synclib/test/test_leaf.o \
./synclib/test/test_pit.o \
./synclib/test/test_scheduler.o \
./synclib/test/test_sync_logic.o 

CC_DEPS += \
./synclib/test/test_app_socket.d \
./synclib/test/test_ccnx_wrapper.d \
./synclib/test/test_data_fetch_and_publish.d \
./synclib/test/test_digest.d \
./synclib/test/test_interest_table.d \
./synclib/test/test_leaf.d \
./synclib/test/test_pit.d \
./synclib/test/test_scheduler.d \
./synclib/test/test_sync_logic.d 


# Each subdirectory must supply rules for building sources it contributes
synclib/test/%.o: ../synclib/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


