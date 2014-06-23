################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../sync-chat/extensions/custom-apps/SimpleRNG.cpp 

CC_SRCS += \
../sync-chat/extensions/custom-apps/custom-app.cc \
../sync-chat/extensions/custom-apps/digest-node.cc \
../sync-chat/extensions/custom-apps/digest-tree.cc \
../sync-chat/extensions/custom-apps/digest.cc \
../sync-chat/extensions/custom-apps/sync-audio-pipe-app.cc \
../sync-chat/extensions/custom-apps/sync-chat-app.cc \
../sync-chat/extensions/custom-apps/sync-chat-start.cc \
../sync-chat/extensions/custom-apps/sync-client.cc \
../sync-chat/extensions/custom-apps/sync-controller.cc \
../sync-chat/extensions/custom-apps/sync-proxy.cc \
../sync-chat/extensions/custom-apps/sync-video-pipe-app.cc 

OBJS += \
./sync-chat/extensions/custom-apps/SimpleRNG.o \
./sync-chat/extensions/custom-apps/custom-app.o \
./sync-chat/extensions/custom-apps/digest-node.o \
./sync-chat/extensions/custom-apps/digest-tree.o \
./sync-chat/extensions/custom-apps/digest.o \
./sync-chat/extensions/custom-apps/sync-audio-pipe-app.o \
./sync-chat/extensions/custom-apps/sync-chat-app.o \
./sync-chat/extensions/custom-apps/sync-chat-start.o \
./sync-chat/extensions/custom-apps/sync-client.o \
./sync-chat/extensions/custom-apps/sync-controller.o \
./sync-chat/extensions/custom-apps/sync-proxy.o \
./sync-chat/extensions/custom-apps/sync-video-pipe-app.o 

CC_DEPS += \
./sync-chat/extensions/custom-apps/custom-app.d \
./sync-chat/extensions/custom-apps/digest-node.d \
./sync-chat/extensions/custom-apps/digest-tree.d \
./sync-chat/extensions/custom-apps/digest.d \
./sync-chat/extensions/custom-apps/sync-audio-pipe-app.d \
./sync-chat/extensions/custom-apps/sync-chat-app.d \
./sync-chat/extensions/custom-apps/sync-chat-start.d \
./sync-chat/extensions/custom-apps/sync-client.d \
./sync-chat/extensions/custom-apps/sync-controller.d \
./sync-chat/extensions/custom-apps/sync-proxy.d \
./sync-chat/extensions/custom-apps/sync-video-pipe-app.d 

CPP_DEPS += \
./sync-chat/extensions/custom-apps/SimpleRNG.d 


# Each subdirectory must supply rules for building sources it contributes
sync-chat/extensions/custom-apps/%.o: ../sync-chat/extensions/custom-apps/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

sync-chat/extensions/custom-apps/%.o: ../sync-chat/extensions/custom-apps/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


