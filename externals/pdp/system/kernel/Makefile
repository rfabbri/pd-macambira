
OBJECTS = pdp.o pdp_ut.o pdp_packet.o pdp_queue.o pdp_comm.o \
	pdp_control.o pdp_compat.o pdp_type.o pdp_dpd_command.o \
	pdp_list.o pdp_forth.o


include ../../Makefile.config

all: pdp_main_clean $(OBJECTS)

pdp_main_clean:
	rm -f pdp.o

clean:
	rm -f *~
	rm -f *.o
