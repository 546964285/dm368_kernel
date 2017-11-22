#############################################################
# Makefile for ARMVectors files, used to generate binary    #
#   ARM executable data for use in the dMax load images.    #
#############################################################
# Author:   Daniel Allred
#

CROSSCOMPILE=470
CC=cl$(CROSSCOMPILE)
OBJDUMP=dis$(CROSSCOMPILE)
IMG2C=./bin/img2c
LINKERFLAGS=-z -e reset
OBJDUMPFLAGS=-q -S -I -a --ual

IDLE_SOURCE=arm_idle_vectors.asm
ROM_SOURCE=arm_rom_vectors.asm

IDLE_CFILE:=$(patsubst %.asm,%.c,$(IDLE_SOURCE))
IDLE_OUT:=$(patsubst %.asm,%.out,$(IDLE_SOURCE))
ROM_CFILE:=$(patsubst %.asm,%.c,$(ROM_SOURCE))
ROM_OUT:=$(patsubst %.asm,%.out,$(ROM_SOURCE))
LINKERSCRIPT:=arm_vectors.cmd

# Target definitions
all: $(IDLE_CFILE) $(ROM_CFILE) $(IDLE_OUT) $(ROM_OUT)

.PHONY : clean objclean
clean:
		-rm -f -v $(IDLE_CFILE) $(ROM_CFILE) $(IDLE_OUT) $(ROM_OUT)

objclean:
		-rm -f -v $(IDLE_OUT) $(ROM_OUT)

%.c: %.img
		$(IMG2C) $(patsubst %.img,%,$< ) $< > $@
    
%.img: %.out
		$(OBJDUMP) $(OBJDUMPFLAGS) $< > $@

%.out : %.obj $(LINKERSCRIPT)
		$(CC) $(LINKERFLAGS) -o $@ $(LINKERSCRIPT) $<

%.obj : %.asm
		$(CC) $<   