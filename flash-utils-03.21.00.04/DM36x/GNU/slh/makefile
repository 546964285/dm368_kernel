#############################################################
# Makefile for TI Serial Loader Host (SLH) project.         #
#   Generates the Host executable file that is used in      #
#   in conjunction with the SLT to send and boot over UART. #
#   This executable is dependent on the SLT binary having   #
#   been generated first.                                   #
#############################################################
#	    

include ../../device.mak

PROGRAM:=slh
MONOCOMPILE=gmcs
DOTNETCOMPILE=csc
REMOTESOURCES=$(PROGRAM).cs device_name.cs Debug.cs CRC.cs EmbeddedFileIO.cs FileIO.cs SerialIO.cs ConsoleUtility.cs

OBJECTS:=$(patsubst %.cs,%.module,$(REMOTESOURCES))
EXECUTABLE:=../$(PROGRAM)_$(DEVSTRING).exe
COMMONPATH:=../../../Common
ICONPATH:=$(COMMONPATH)/ti.ico
UTILLIBPATH:=$(COMMONPATH)/UtilLib
SLTIMAGES=$(wildcard ../slt/slt_*.bin)

# Target definitions
all: $(EXECUTABLE)

.PHONY : clean slt
clean:
		-rm -f -v $(EXECUTABLE) $(OBJECTS)
slt:
		make -C ../slt all

$(EXECUTABLE): slt $(OBJECTS)
ifeq ($(shell uname),Linux)
		@$(MONOCOMPILE) -t:exe $(patsubst %,-resource:%,$(SLTIMAGES)) -out:$(EXECUTABLE) $(OBJECTS)
else
		@$(DOTNETCOMPILE) /t:exe $(patsubst %,/resource:%,$(SLTIMAGES)) /out:$(EXECUTABLE) /win32icon:$(ICONPATH) $(OBJECTS)
endif

%.module: %.cs
		@cp $< $@


vpath %.cs $(UTILLIBPATH)/:$(COMMONPATH)/slh/:../../Common/src/


