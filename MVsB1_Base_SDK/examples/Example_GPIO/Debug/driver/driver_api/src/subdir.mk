################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/src/uarts_interface.c 

OBJS += \
./driver/driver_api/src/uarts_interface.o 

C_DEPS += \
./driver/driver_api/src/uarts_interface.d 


# Each subdirectory must supply rules for building sources it contributes
driver/driver_api/src/uarts_interface.o: /cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/src/uarts_interface.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/middleware/mv_utils/inc" -Og -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -fsingle-precision-constant -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


