current: all_modules

include ../../Makefile.config

PDP_MOD = pdp_reg.o pdp_del.o pdp_snap.o pdp_trigger.o \
	pdp_route.o pdp_inspect.o pdp_loop.o pdp_description.o pdp_convert.o \
	pdp_forthproc.o

# build generic modules
all_modules: $(PDP_MOD)

clean:
	rm -f *~
	rm -f *.o

