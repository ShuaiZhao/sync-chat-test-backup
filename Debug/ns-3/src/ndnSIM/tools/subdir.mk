################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../ns-3/src/ndnSIM/tools/rocketfuel-maps-cch-to-annotaded.cc 

OBJS += \
./ns-3/src/ndnSIM/tools/rocketfuel-maps-cch-to-annotaded.o 

CC_DEPS += \
./ns-3/src/ndnSIM/tools/rocketfuel-maps-cch-to-annotaded.d 


# Each subdirectory must supply rules for building sources it contributes
ns-3/src/ndnSIM/tools/%.o: ../ns-3/src/ndnSIM/tools/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


