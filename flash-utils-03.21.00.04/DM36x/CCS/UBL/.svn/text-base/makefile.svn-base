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
    		for j in $(DEVICETYPES); do \
						echo -D$${j} > "devtype.txt"; \
						echo "Building $(PROGRAM)_$${i}.pjt for device $${j}"; \
						timake $(PROGRAM)_$${i}.pjt Debug -a; \
						mv $(PROGRAM).bin $(PROGRAM)_$${j}_$${i}.bin; \
				done \
		done
#Save a devtype.txt with default device type
		@echo -D$(word 1,$(DEVICETYPES)) > "devtype.txt"

clean:
		@echo "Cleaning projects..."
		@for i in $(FLASHTYPES); do \
    		for j in $(DEVICETYPES); do \
						timake $(PROGRAM)_$${i}.pjt Debug -clean; \
				done \
		done
		@echo "Removing binaries..."
		@for i in $(FLASHTYPES); do \
    		for j in $(DEVICETYPES); do \
						rm -f $(PROGRAM)_$${j}_$${i}.bin; \
				done \
		done
