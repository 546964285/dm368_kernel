#############################################################
# Makefile for UBL CCS project(s).                          #
#############################################################
#

include ../../device.mak
PROGRAM:=UBL

# Target definitions
.PHONY : all clean
all:
		@for i in $(FLASHTYPES); do \
    		for j in $(DSP_DEVICETYPES); do \
						echo --define=$${j} > "devtype.txt"; \
						echo "Building $(PROGRAM)_$${i}.pjt for device $${j}"; \
						timake $(PROGRAM)_$${i}.pjt Debug -a; \
						mv $(PROGRAM)_$${i}.ais $(PROGRAM)_$${j}_$${i}.ais; \
				done \
		done
#Save a devtype.txt with default device type
		@echo --define=$(word 1,$(DSP_DEVICETYPES)) > "devtype.txt"

clean:
		@echo "Cleaning projects..."
		@for i in $(FLASHTYPES); do \
    		for j in $(DSP_DEVICETYPES); do \
						timake $(PROGRAM)_$${i}.pjt Debug -clean; \
				done \
		done
		@echo "Removing binaries..."
		@for i in $(FLASHTYPES); do \
    		for j in $(DSP_DEVICETYPES); do \
						rm -f $(PROGRAM)_$${j}_$${i}.ais; \
				done \
		done
