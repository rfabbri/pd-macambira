target:	all_objects

include ../Makefile.config
include Makefile.$(PDP_TARGET)



OBJECTS = pdp.o pdp_ut.o pdp_packet.o pdp_type.o pdp_queue.o pdp_comm.o \
	pdp_control.o pdp_llconv.o pdp_resample.o

pdp_main_clean:
	rm -f pdp.o

all_objects: pdp_main_clean $(OBJECTS) platform_targets

clean:
	rm -f *~
	rm -f *.o
	make -C mmx clean

