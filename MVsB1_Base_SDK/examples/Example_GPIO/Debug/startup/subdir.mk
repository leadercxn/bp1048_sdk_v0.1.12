################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/init-default.c \
C:/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/interrupt.c \
C:/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/retarget.c 

S_UPPER_SRCS += \
C:/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/crt0.S 

OBJS += \
./startup/crt0.o \
./startup/init-default.o \
./startup/interrupt.o \
./startup/retarget.o 

C_DEPS += \
./startup/init-default.d \
./startup/interrupt.d \
./startup/retarget.d 

S_UPPER_DEPS += \
./startup/crt0.d 


# Each subdirectory must supply rules for building sources it contributes
startup/crt0.o: /cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/crt0.S
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/middleware/mv_utils/inc" -Og -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -fsingle-precision-constant -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/init-default.o: /cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/init-default.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/middleware/mv_utils/inc" -Og -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -fsingle-precision-constant -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/interrupt.o: /cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/interrupt.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/middleware/mv_utils/inc" -Og -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -fsingle-precision-constant -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/retarget.o: /cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/startup/retarget.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/driver/driver_api/inc" -I"/cygdrive/C/cxn/andes/bp_sdk/MVsB1_BT_Audio_SDK_v0.1.12/MVsB1_Base_SDK/middleware/mv_utils/inc" -Og -mcmodel=medium -g3 -Wall -mcpu=d1088-spu -c -fmessage-length=0 -ldsp -mext-dsp -fsingle-precision-constant -ffunction-sections -fdata-sections -mext-dsp -mext-zol -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


