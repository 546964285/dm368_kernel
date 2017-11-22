#############################################################
# Makefile for TI Serial Flasher Host (SFH) project.        #
#   Generates the Host executable file that is used in      #
#   in conjunction with the SFT to send and boot over UART. #
#   This executable is dependent on the SFT binary having   #
#   been generated first.                                   #
#############################################################
#

include ../../device.mak

PROGRAM:=sfh
MONOCOMPILE=gmcs
DOTNETCOMPILE=csc
REMOTESOURCES=$(PROGRAM).cs device_name.cs Debug.cs CRC.cs EmbeddedFileIO.cs FileIO.cs SerialIO.cs ConsoleUtility.cs

OBJECTS:=$(patsubst %.cs,%.module,$(REMOTESOURCES))
EXECUTABLE:=../$(PROGRAM)_$(DEVSTRING).exe
COMMONPATH:=../../../Common
ICONPATH:=$(COMMONPATH)/ti.ico
UTILLIBPATH:=$(COMMONPATH)/UtilLib
SFTIMAGES=$(wildcard ../sft/sft_*.bin)

# Target definitions
all: $(EXECUTABLE)

.PHONY : clean sft
clean:
		-rm -f -v $(EXECUTABLE) $(OBJECTS)
sft:
		make -C ../sft all
    
$(EXECUTABLE): sft $(OBJECTS)
ifeq ($(shell uname),Linux)
		@$(MONOCOMPILE) -t:exe $(patsubst %,-resource:%,$(SFTIMAGES)) -out:$(EXECUTABLE) $(OBJECTS)
else
		@$(DOTNETCOMPILE) /t:exe $(patsubst %,/resource:%,$(SFTIMAGES)) /out:$(EXECUTABLE) /win32icon:$(ICONPATH) $(OBJECTS)
endif

%.module: %.cs
		@cp $< $@

vpath %.cs $(UTILLIBPATH)/:$(COMMONPATH)/sfh/:../../Common/src/


