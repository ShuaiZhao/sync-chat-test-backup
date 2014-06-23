################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../synclib/src/sync-diff-leaf.cc \
../synclib/src/sync-diff-state.cc \
../synclib/src/sync-digest.cc \
../synclib/src/sync-full-leaf.cc \
../synclib/src/sync-full-state.cc \
../synclib/src/sync-interest-table.cc \
../synclib/src/sync-leaf.cc \
../synclib/src/sync-logic.cc \
../synclib/src/sync-name-info.cc \
../synclib/src/sync-seq-no.cc \
../synclib/src/sync-state.cc \
../synclib/src/sync-std-name-info.cc 

OBJS += \
./synclib/src/sync-diff-leaf.o \
./synclib/src/sync-diff-state.o \
./synclib/src/sync-digest.o \
./synclib/src/sync-full-leaf.o \
./synclib/src/sync-full-state.o \
./synclib/src/sync-interest-table.o \
./synclib/src/sync-leaf.o \
./synclib/src/sync-logic.o \
./synclib/src/sync-name-info.o \
./synclib/src/sync-seq-no.o \
./synclib/src/sync-state.o \
./synclib/src/sync-std-name-info.o 

CC_DEPS += \
./synclib/src/sync-diff-leaf.d \
./synclib/src/sync-diff-state.d \
./synclib/src/sync-digest.d \
./synclib/src/sync-full-leaf.d \
./synclib/src/sync-full-state.d \
./synclib/src/sync-interest-table.d \
./synclib/src/sync-leaf.d \
./synclib/src/sync-logic.d \
./synclib/src/sync-name-info.d \
./synclib/src/sync-seq-no.d \
./synclib/src/sync-state.d \
./synclib/src/sync-std-name-info.d 


# Each subdirectory must supply rules for building sources it contributes
synclib/src/%.o: ../synclib/src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


