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
REMOTESOURCES=$(PROGRAM)_ais.cs AIS.cs AISParse.cs device_name.cs Debug.cs CRC.cs EmbeddedFileIO.cs FileIO.cs SerialIO.cs ConsoleUtility.cs

OBJECTS:=$(patsubst %.cs,%.module,$(REMOTESOURCES))
EXECUTABLE:=../$(PROGRAM)_$(DEVSTRING).exe
COMMONPATH:=../../../Common
ICONPATH:=$(COMMONPATH)/ti.ico
UTILLIBPATH:=$(COMMONPATH)/UtilLib

# Target definitions
all: $(EXECUTABLE)

.PHONY : clean
clean:
		-rm -f -v $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS) $(SLTIMAGE)
ifeq ($(shell uname),Linux)
		@$(MONOCOMPILE) -t:exe -out:$(EXECUTABLE) $(OBJECTS)
else
		@$(DOTNETCOMPILE) /t:exe /out:$(EXECUTABLE) /win32icon:$(ICONPATH) $(OBJECTS)
endif

%.module: %.cs
		@cp $< $@

vpath %.cs $(UTILLIBPATH)/:$(COMMONPATH)/AIS/:$(COMMONPATH)/slh/:../../Common/src/


