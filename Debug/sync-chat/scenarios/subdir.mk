################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../sync-chat/scenarios/chat-app-test.cc \
../sync-chat/scenarios/example1.cc \
../sync-chat/scenarios/example2.cc \
../sync-chat/scenarios/example3.cc \
../sync-chat/scenarios/sync-scheme.cc \
../sync-chat/scenarios/sync-simulation.cc \
../sync-chat/scenarios/topo-gen-example.cc 

OBJS += \
./sync-chat/scenarios/chat-app-test.o \
./sync-chat/scenarios/example1.o \
./sync-chat/scenarios/example2.o \
./sync-chat/scenarios/example3.o \
./sync-chat/scenarios/sync-scheme.o \
./sync-chat/scenarios/sync-simulation.o \
./sync-chat/scenarios/topo-gen-example.o 

CC_DEPS += \
./sync-chat/scenarios/chat-app-test.d \
./sync-chat/scenarios/example1.d \
./sync-chat/scenarios/example2.d \
./sync-chat/scenarios/example3.d \
./sync-chat/scenarios/sync-scheme.d \
./sync-chat/scenarios/sync-simulation.d \
./sync-chat/scenarios/topo-gen-example.d 


# Each subdirectory must supply rules for building sources it contributes
sync-chat/scenarios/%.o: ../sync-chat/scenarios/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


