
OBJECTS = pdp.o pdp_ut.o pdp_packet.o pdp_type.o pdp_queue.o pdp_comm.o \
	pdp_control.o pdp_llconv.o pdp_resample.o

OBJECTS_MMX = pdp_imageproc_mmx.o pdp_llconv_mmx.o
OBJECTS_PORTABLE =  pdp_imageproc_portable.o pdp_llconv_portable.o



include ../Makefile.config

all: $(PDP_TARGET) common_objects


linux_mmx: $(OBJECTS_MMX)
	make -C mmx

linux: $(OBJECTS_PORTABLE)

darwin: $(OBJECTS_PORTABLE)

pdp_main_clean:
	rm -f pdp.o

common_objects: pdp_main_clean $(OBJECTS)

clean:
	rm -f *~
	rm -f *.o
	make -C mmx clean
