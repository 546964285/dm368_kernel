include ../../device.mak

all:
		@for i in $(DEVICETYPES); do \
				$(MAKE) -C build DEVICETYPE=$${i} all; \
		done

clean:
		@for i in $(DEVICETYPES); do \
				$(MAKE) -C build DEVICETYPE=$${i} clean; \
		done
%::
		@for i in $(DEVICETYPES); do \
				$(MAKE) -C build DEVICETYPE=$${j} $@; \
		done

