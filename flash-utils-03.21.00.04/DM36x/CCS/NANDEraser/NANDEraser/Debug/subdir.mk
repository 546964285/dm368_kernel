################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
Z:/fix/cat_flashutils/Common/drivers/src/async_mem.c \
Z:/fix/cat_flashutils/Common/ccs/src/debug.c \
Z:/fix/cat_flashutils/DM36x/Common/src/device.c \
Z:/fix/cat_flashutils/DM36x/Common/src/device_async_mem.c \
Z:/fix/cat_flashutils/DM36x/Common/src/device_nand.c \
Z:/fix/cat_flashutils/Common/drivers/src/nand.c \
Z:/fix/cat_flashutils/Common/src/util.c 

CMD_SRCS += \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/NANDEraser.cmd 

OBJS += \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/async_mem.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/debug.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device_async_mem.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device_nand.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/nand.obj \
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/util.obj 

C_DEPS += \
./async_mem.pp \
./debug.pp \
./device.pp \
./device_async_mem.pp \
./device_nand.pp \
./nand.pp \
./util.pp 

C_DEPS__QUOTED += \
"async_mem.pp" \
"debug.pp" \
"device.pp" \
"device_async_mem.pp" \
"device_nand.pp" \
"nand.pp" \
"util.pp" 

OBJS__QUOTED += \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\async_mem.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\debug.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\device.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\device_async_mem.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\device_nand.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\nand.obj" \
"Z:\fix\cat_flashutils\DM36x\CCS\NANDEraser\Debug\util.obj" 

C_SRCS__QUOTED += \
"Z:/fix/cat_flashutils/Common/drivers/src/async_mem.c" \
"Z:/fix/cat_flashutils/Common/ccs/src/debug.c" \
"Z:/fix/cat_flashutils/DM36x/Common/src/device.c" \
"Z:/fix/cat_flashutils/DM36x/Common/src/device_async_mem.c" \
"Z:/fix/cat_flashutils/DM36x/Common/src/device_nand.c" \
"Z:/fix/cat_flashutils/Common/drivers/src/nand.c" \
"Z:/fix/cat_flashutils/Common/src/util.c" 


# Each subdirectory must supply rules for building sources it contributes
Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/async_mem.obj: Z:/fix/cat_flashutils/Common/drivers/src/async_mem.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="async_mem.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/debug.obj: Z:/fix/cat_flashutils/Common/ccs/src/debug.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="debug.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device.obj: Z:/fix/cat_flashutils/DM36x/Common/src/device.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="device.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device_async_mem.obj: Z:/fix/cat_flashutils/DM36x/Common/src/device_async_mem.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="device_async_mem.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/device_nand.obj: Z:/fix/cat_flashutils/DM36x/Common/src/device_nand.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="device_nand.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/nand.obj: Z:/fix/cat_flashutils/Common/drivers/src/nand.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="nand.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug/util.obj: Z:/fix/cat_flashutils/Common/src/util.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/bin/cl470" -mv5e -g --define="_DEBUG" --define="SKIP_LOW_LEVEL_INIT" --include_path="C:/Program Files/Texas Instruments/ccsv5/tools/compiler/tms470/include" --include_path="C:/Program Files/Texas Instruments/ccsv5/xdais_7_20_00_07/packages/ti/xdais" --include_path="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/include" --include_path="Z:/fix/cat_flashutils/Common/ccs/include" --include_path="Z:/fix/cat_flashutils/DM36x/Common/include" --include_path="Z:/fix/cat_flashutils/Common/include" --include_path="Z:/fix/cat_flashutils/Common/drivers/include"  --diag_warning=225 -me --abi=eabi --obj_directory="Z:/fix/cat_flashutils/DM36x/CCS/NANDEraser/Debug"  --preproc_with_compile --preproc_dependency="util.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


