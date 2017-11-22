#
# Source: /dsps_rbl/legacy/OMAP-L138/trunk/ROM/Boot_Loader/dMaxARMLoad/code/makefile
#
# dMaxARMLoad Makefile 
#
# Copyright 2008, Texas Instruments.  All rights reserved.
#

PRU_TOOLS_PATH=../bin
PRU_ASM=pasm
PRU_SRC=PRU_mem1DTransfer.p
PRU0_IMAGE=$(patsubst %.p,%_bin.h,$(PRU_SRC))
PRU0_C_FILE=../PRU_mem1DTransfer.c
PRU0_ARRAY_NAME=pru_mem1d_transfer

all: 	c_file

.PHONY : clean

clean:
	rm -f ${PRU0_C_FILE}

c_file: ${PRU0_C_FILE}

$(PRU0_C_FILE): $(PRU0_IMAGE)
	@echo "#pragma DATA_SECTION($(PRU0_ARRAY_NAME),\".const:$(PRU0_ARRAY_NAME)\")" > $@
	@cat $@ ${PRU0_IMAGE} > temp.c
	@cp temp.c $@
	@rm -f *.c
	@rm -f ${PRU0_IMAGE}
	@echo "Done generating PRU0 image for ARM vector loading..."

$(PRU0_IMAGE): $(PRU_SRC)
	@echo "Generating ARM vector load PDSP0 image..."
	@$(PRU_TOOLS_PATH)/$(PRU_ASM) -c -C$(PRU0_ARRAY_NAME) $<


