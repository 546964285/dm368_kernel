#############################################################
# Makefile for ConsoleUtility Test case.                    #
#############################################################
#	    

PROGRAM:=ConsoleUtilityTest
MONOCOMPILE=gmcs
DOTNETCOMPILE=csc
LOCALSOURCES=$(PROGRAM).cs
REMOTESOURCES=ConsoleUtility.cs

OBJECTS:=$(patsubst %.cs,%.module,$(REMOTESOURCES))
EXECUTABLE:=../$(PROGRAM).exe
COMMONPATH:=../../../../Common
ICONPATH:=$(COMMONPATH)/ti.ico
UTILLIBPATH:=$(COMMONPATH)/UtilLib

# Target definitions
all: $(EXECUTABLE)

.PHONY : clean
clean:
		-rm -f -v $(EXECUTABLE) $(OBJECTS)
    
$(EXECUTABLE): $(LOCALSOURCES) $(OBJECTS)
ifeq ($(shell uname),Linux)
		@$(MONOCOMPILE) -t:exe $(patsubst %,-resource:%,$(SFTIMAGES)) -out:$(EXECUTABLE) $(LOCALSOURCES) $(OBJECTS)
else
		@$(DOTNETCOMPILE) /t:exe $(patsubst %,/resource:%,$(SFTIMAGES)) /out:$(EXECUTABLE) /win32icon:$(ICONPATH) $(LOCALSOURCES) $(OBJECTS)
endif

%.module: %.cs
		@cp $< $@

vpath %.cs $(UTILLIBPATH)/:$(COMMONPATH)/AIS/:$(COMMONPATH)/sfh/:../../Common/src/


