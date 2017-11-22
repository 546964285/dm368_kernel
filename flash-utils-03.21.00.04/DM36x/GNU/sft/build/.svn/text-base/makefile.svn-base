#############################################################
# Makefile for TI Serial Flash Target (SFT) project.        #
#   Generates the binary SFT file that is intended to be    #
#   executed on the device for writing the flash storage.   #
#############################################################
#

# CROSSCOMPILE definition for entire tree
include ../../../../Common/build.mak

PROGRAM:=sft
CC=$(CROSSCOMPILE)gcc
OBJCOPY=$(CROSSCOMPILE)objcopy
OBJDUMP=$(CROSSCOMPILE)objdump
LDFLAGS=-Wl,-T$(LINKERSCRIPT) -nostdlib 
OBJCOPYFLAGS = --gap-fill 0xFF -S
SOURCES=$(PROGRAM).c selfcopy.c boot.c uartboot.c device.c debug.c uart.c device_uart.c util.c
ENTRYPOINT=boot

CFLAGS:=-c -Os -Wall -ffreestanding -DUBL_$(FLASHTYPE) -D$(DEVICETYPE) -I../../../Common/include -I../../../../Common/include -I../../../../Common/arch/arm926ejs/include -I../../../../Common/$(PROGRAM)/include -I../../../../Common/ubl/include -I../../../../Common/drivers/include -I../../../../Common/gnu/include 
ifeq ($(FLASHTYPE),NAND)
  SOURCES+= async_mem.c device_async_mem.c nand.c device_nand.c
endif
ifeq ($(FLASHTYPE),NOR)
  SOURCES+= async_mem.c device_async_mem.c nor.c
endif
ifeq ($(FLASHTYPE),SDMMC)
  SOURCES+= sdmmc.c
endif
ifeq ($(FLASHTYPE),ONENAND)
  SOURCES+= async_mem.c device_async_mem.c onenand.c
endif
ifeq ($(FLASHTYPE),I2C_MEM)
  SOURCES+= i2c.c i2c_mem.c device_i2c.c
endif
ifeq ($(FLASHTYPE),SPI_MEM)
  SOURCES+= spi.c spi_mem.c device_spi.c
endif

LINKERSCRIPT=../$(PROGRAM).lds
OBJECTS:=$(patsubst %.c,%_$(DEVICETYPE)_$(FLASHTYPE).o,$(SOURCES))
EXECUTABLE:=$(PROGRAM)_$(DEVICETYPE)_$(FLASHTYPE)
BINARY:=../$(EXECUTABLE).bin

# Target definitions
all: $(BINARY) $(OBJECTS)

.PHONY : clean objclean
clean:
		-rm -f -v $(OBJECTS) $(BINARY) $(EXECUTABLE)
		
objclean:
		-rm -f -v $(OBJECTS)
		
$(BINARY): $(EXECUTABLE)
		$(OBJCOPY) $(OBJCOPYFLAGS) -O binary $< $@
		@echo -n "The entry point of the binary is 0x"
		@$(OBJDUMP) -t $< | grep -E '[0-9a-f]{8}*[0-9a-f]{8} $(ENTRYPOINT)' | sed  's/^\([0-9a-f]\{4\}\)\([0-9a-f]\{4\}\).*/\2/'

$(EXECUTABLE): $(OBJECTS) $(LINKERSCRIPT)
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@
		
%_$(DEVICETYPE)_$(FLASHTYPE).o: %.c %.h
		$(CC) $(CFLAGS) $< -o $@      
        
# Search paths for code and header files
vpath %.h :../../../Common/include:../../../../Common/include:../../../../Common/arch/arm926ejs/include:../../../../Common/$(PROGRAM)/include:../../../../Common/ubl/include:../../../../Common/gnu/include:../../../../Common/drivers/include
vpath %.c ../../../Common/src:../../../../Common/src:../../../../Common/arch/arm926ejs/src:../../../../Common/$(PROGRAM)/src:../../../../Common/ubl/src:../../../../Common/gnu/src:../../../../Common/drivers/src
vpath %.asm ../../../../Common/arch/arm926ejs:../../../../Common/arch/c6000
