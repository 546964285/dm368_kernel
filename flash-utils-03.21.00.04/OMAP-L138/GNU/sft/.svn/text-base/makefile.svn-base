include ../../device.mak

all:
		@for i in $(FLASHTYPES); do \
				for j in $(DEVICETYPES); do \
						$(MAKE) -C build FLASHTYPE=$${i} DEVICETYPE=$${j} all; \
				done \
		done
clean:
		@for i in $(FLASHTYPES); do \
				for j in $(DEVICETYPES); do \
						$(MAKE) -C build FLASHTYPE=$${i} DEVICETYPE=$${j} clean; \
				done \
		done
%::
		@for i in $(FLASHTYPES); do \
				for j in $(DEVICETYPES); do \
						$(MAKE) -C build FLASHTYPE=$${i} DEVICETYPE=$${j} $@; \
				done \
		done
		
