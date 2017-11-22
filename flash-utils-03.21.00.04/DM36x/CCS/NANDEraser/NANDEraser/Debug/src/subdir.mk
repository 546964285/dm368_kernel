################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/src/nanderaser.c 

OBJS += \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/nanderaser.obj 

C_DEPS += \
./src/nanderaser.pp 

C_DEPS__QUOTED += \
"src\nanderaser.pp" 

OBJS__QUOTED += \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\nanderaser.obj" 

C_SRCS__QUOTED += \
"Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/src/nanderaser.c" 


# Each subdirectory must supply rules for building sources it contributes
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/nanderaser.obj: Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/src/nanderaser.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="src/nanderaser.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


