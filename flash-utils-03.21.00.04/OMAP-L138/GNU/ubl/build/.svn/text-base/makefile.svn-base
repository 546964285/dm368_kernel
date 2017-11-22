#############################################################
# Makefile for TI UBL project.                              #
#   Generates the binary UBL file can be used as the        #
#   secondary bootloader for the OMAP-L138 in NOR, NAND,    #
#   and UART boot modes.                                    #
#############################################################
# Author:   Jeff Cobb, Daniel Allred
#

# CROSSCOMPILE definition for entire tree
include ../../../../Common/build.mak
include ../../../device.mak

PROGRAM:=ubl
SOURCES=$(PROGRAM).c device.c uartboot.c device_uart.c uart.c debug.c util.c tps65070.c i2c.c

CFLAGS=-c -g -DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -I=../../../Common/include -I=../../../../Common/include -I=../../../../Common/arch/c6000/include -I=../../../../Common/$(PROGRAM)/include -I=../../../../Common/ubl/include -I=../../../../Common/drivers/include -I=../../../../Common/gnu/include -I=../../../../Common/ubl/include -DAIS_RBL -DUBL -DUSE_IN_ROM -DDEVICE_UART2_FOR_DEBUG -DOPERATING_POINT -DCONFIG_MACH_DAVINCI_DA850_EVM

CFLAGS_GCC=-DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -I../../../Common/include -I../../../../Common/include -I../../../../Common/arch/c6000/include -I../../../../Common/$(PROGRAM)/include -I../../../../Common/ubl/include -I../../../../Common/drivers/include -I../../../../Common/gnu/include -I../../../../Common/ubl/include -I../../../../Common/arch/arm926ejs/include -DAIS_RBL -DUBL -DUSE_IN_ROM -DDEVICE_UART2_FOR_DEBUG -DOPERATING_POINT -DCONFIG_MACH_DAVINCI_DA850_EVM

LNKFLAGS=-z -c -estart -a -w -x  -o=$@ $(LINKERSCRIPT) $(OBJECTS)
LNKFLAG_GCC=-Wl,-T$(LINKERSCRIPT) -nostdlib 
AFLAGS=

ifeq ($(FLASHTYPE),NAND)
  SOURCES+= async_mem.c device_async_mem.c nand.c device_nand.c nandboot.c
endif
ifeq ($(FLASHTYPE),NOR)
  SOURCES+= async_mem.c device_async_mem.c nor.c norboot.c
endif
ifeq ($(FLASHTYPE),SDMMC)
  SOURCES+= sdmmc.c sdmmcboot.c
endif
ifeq ($(FLASHTYPE),ONENAND)
  SOURCES+= async_mem.c device_async_mem.c onenand.c onenandboot.c
endif
ifeq ($(FLASHTYPE),I2C_MEM)
  SOURCES+= i2c.c i2c_mem.c device_i2c.c i2c_memboot.c
endif
ifeq ($(FLASHTYPE),SPI_MEM)
  SOURCES+= spi.c spi_mem.c device_spi.c spi_memboot.c
endif

ifeq ($(DEVICETYPE),OMAPL138)
  CC=$(CROSSCOMPILE)gcc
  SOURCES+= start.asm
  CFLAGS = $(CFLAGS_GCC) -c -Os -Wall -ffreestanding -DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -o $@   
  LNKFLAGS = $(LNKFLAG_GCC) -o $@ $(LINKERSCRIPT) $(OBJECTS)
  LINKERSCRIPT:=../ARM_$(PROGRAM).lds
  AFLAGS = -x assembler
endif
ifeq ($(DEVICETYPE),AM1808)
  CC=$(CROSSCOMPILE)gcc
  SOURCES+= start.asm
  CFLAGS = $(CFLAGS_GCC) -c -Os -Wall -ffreestanding -DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -o $@   
  LNKFLAGS = $(LNKFLAG_GCC) -o $@ $(LINKERSCRIPT) $(OBJECTS)
  LINKERSCRIPT:=../ARM_$(PROGRAM).lds
  AFLAGS = -x assembler
endif
ifeq ($(DEVICETYPE),INTDEV0)
  CC=$(CROSSCOMPILE)gcc
  SOURCES+= start.asm
  CFLAGS = $(CFLAGS_GCC) -c -Os -Wall -ffreestanding -DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -o $@   
  LNKFLAGS = $(LNKFLAG_GCC) -o $@ $(LINKERSCRIPT) $(OBJECTS)
  LINKERSCRIPT:=../ARM_$(PROGRAM).lds
  AFLAGS = -x assembler
endif
ifeq ($(DEVICETYPE),C6748)
  CC=cl6x
  SOURCES+= start_c674x.asm pru.c
  CFLAGS+= -ms3 -mo -mv6740 --obj_extension=$(DEVICETYPE)_$(FLASHTYPE).obj 
  LINKERSCRIPT:=../DSP_$(PROGRAM).cmd
endif
ifeq ($(DEVICETYPE),C6746)
  CC=cl6x
  SOURCES+= start_c674x.asm pru.c
  CFLAGS+= -ms3 -mo -mv6740 --obj_extension=$(DEVICETYPE)_$(FLASHTYPE).obj 
  LINKERSCRIPT:=../DSP_$(PROGRAM).cmd
endif

OBJECTS:=$(patsubst %.c,%.$(DEVICETYPE)_$(FLASHTYPE).obj,$(SOURCES))
OBJECTS:=$(patsubst %.asm,%.$(DEVICETYPE)_$(FLASHTYPE).obj,$(OBJECTS))
EXECUTABLE:=../$(PROGRAM)_$(DEVICETYPE)_$(FLASHTYPE).out
AIS_OUTPUT:=$(patsubst %.out,%.bin,$(EXECUTABLE))

ifeq ($(shell uname),Linux)
	HEX_AIS_EXECUTABLE:=mono ../../AISUtils/HexAIS_$(DEVSTRING).exe
else
	HEX_AIS_EXECUTABLE:=../../AISUtils/HexAIS_$(DEVSTRING).exe
endif

ifeq ($(FLASHTYPE),NOR)
  HEX_AIS_INI:=../$(PROGRAM)_nor_hexais.ini
else
  HEX_AIS_INI:=../$(PROGRAM)_hexais.ini
endif

all: $(AIS_OUTPUT) $(LINKERSCRIPT) makefile

.PHONY : clean objclean
clean:
		-rm -f -v $(OBJECTS) $(EXECUTABLE) $(AIS_OUTPUT)

objclean:
		-rm -f -v $(OBJECTS)

$(AIS_OUTPUT):$(EXECUTABLE)
		$(HEX_AIS_EXECUTABLE) -ini $(HEX_AIS_INI) -o $@ $< 
    
$(EXECUTABLE): $(OBJECTS)
		$(CC) $(LNKFLAGS)

%.$(DEVICETYPE)_$(FLASHTYPE).obj : %.asm
		$(CC) $(CFLAGS) $(AFLAGS) $<
    
%.$(DEVICETYPE)_$(FLASHTYPE).obj : %.c
		$(CC) $(CFLAGS) $< 

vpath %.h ../../../Common/include:../../../../Common/include:../../../../Common/arch/arm926ejs/include:../../../../Common/$(PROGRAM)/include:../../../../Common/ubl/include:../../../../Common/gnu/include:../../../../Common/drivers/include
vpath %.c ../../../Common/src:../../../../Common/src:../../../../Common/arch/arm926ejs/src:../../../../Common/$(PROGRAM)/src:../../../../Common/ubl/src:../../../../Common/gnu/src:../../../../Common/drivers/src
vpath %.asm ../../../../Common/arch/arm926ejs:../../../../Common/arch/c6000
