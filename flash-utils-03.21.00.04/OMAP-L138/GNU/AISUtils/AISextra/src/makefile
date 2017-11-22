#############################################################
# Makefile for TI AISExtra project.                         #
#   Generates the COFF AISExtra file which is used to       #
#   supplement the AIS ROM bootloader                       #
#############################################################
# Auth: Daniel Allred
#
PROGRAM:=AISExtra

ARM_CC=arm-none-linux-gnueabi-gcc

SOURCES=aisextra.c
DEVSTRING=OMAP-L138



ARM_OBJECTS:=$(patsubst %.c,./arm_obj/%.obj,$(SOURCES))
ARM_EXECUTABLE:=../ARM_$(PROGRAM)_$(DEVSTRING).out
ARM_LINKERSCRIPT:=ARM_AISExtra.lds

ARM_CFLAGS= -I../../../../../Common/include -I../../../../Common/include -I../../../Common/include -I../include -c -Os -Wall -ffreestanding -o $@
ARM_LNKFLAGS=-Wl,-T$(ARM_LINKERSCRIPT) -nostdlib -u PatchDDRConfig


all: $(ARM_EXECUTABLE) $(ARM_OBJECTS) 

.PHONY : clean objclean
clean:
		-rm -f -v $(ARM_OBJECTS) $(ARM_EXECUTABLE)
		
objclean:
		-rm -f -v $(ARM_OBJECTS)

    
$(ARM_EXECUTABLE): $(ARM_OBJECTS) $(ARM_LINKERSCRIPT)
		$(ARM_CC) $(ARM_LNKFLAGS) $(ARM_OBJECTS) -o $@ 

./arm_obj/%.obj : %.c
		$(ARM_CC) $(ARM_CFLAGS) $<

vpath %.c ../../../../../Common/src:../../../../Common/src:../../../Common/src:../../Common/src:../src        
vpath %.h ../../../../../Common/include:../../../../Common/include:../../../Common/include:../../Common/include:../include       
