#############################################################
# Makefile for TI Non-Secure HexAIS project.                #
#############################################################
# Auth: Daniel Allred
#	    
include ../../../device.mak

PROGRAM:=HexAIS

MONOCOMPILE=gmcs
DOTNETCOMPILE=csc
LOCALSOURCES=$(PROGRAM).cs
REMOTESOURCES=device_name.cs AISGen_$(DEVSTRING).cs Ini.cs CRC.cs Endian.cs $(PROGRAM)_version.cs \
              EmbeddedFileIO.cs Debug.cs HexConv.cs AIS.cs AISGen.cs SerialIO.cs ObjectFile.cs \
              CoffFile.cs ElfFile.cs BinaryFile.cs
OBJECTS:=$(patsubst %.cs,%.module,$(REMOTESOURCES))
EXECUTABLE:=../$(PROGRAM)_$(DEVSTRING).exe

COMMONPATH:=../../../../Common
ICON:=$(COMMONPATH)/ti.ico
UTILLIBPATH:=$(COMMONPATH)/UtilLib
AISGENLIBPATH:=$(COMMONPATH)/AIS

DEVCOMMONPATH:=../../../Common
GNUCOMMONPATH:=../../Common
DEVAISGENLIBPATH:=$(GNUCOMMONPATH)/AISGenLib

AISEXTRAPATH:=../AISextra
ARM_AISEXTRAIMAGE:=$(AISEXTRAPATH)/ARM_AISExtra_$(DEVSTRING).out

# Target definitions
all: $(EXECUTABLE) $(OBJECTS) $(LOCALSOURCES) $(REMOTESOURCES) 

.PHONY : clean
clean:
		-rm -f -v $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(LOCALSOURCES) $(OBJECTS) $(REMOTESOURCES) makefile
ifeq ($(shell uname),Linux)
		@$(MONOCOMPILE) -t:exe -resource:$(ARM_AISEXTRAIMAGE) -out:$(EXECUTABLE) $(LOCALSOURCES) $(OBJECTS)
else
		@$(DOTNETCOMPILE) /t:exe /resource:$(ARM_AISEXTRAIMAGE) /resource:$(DSP_AISEXTRAIMAGE) /out:$(EXECUTABLE) /win32icon:$(ICON) $(LOCALSOURCES) $(OBJECTS)
endif

%.module: %.cs
		@cp $< $@

vpath %.cs $(UTILLIBPATH)/:$(DEVAISGENLIBPATH)/:$(AISGENLIBPATH)/:$(DEVCOMMONPATH)/src/
