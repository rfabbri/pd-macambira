current: all_modules

include ../Makefile.config

OBJECTS = pdp_xv.o pdp_qt.o pdp_add.o pdp_reg.o pdp_conv.o \
	pdp_mix.o pdp_v4l.o pdp_affine.o pdp_del.o pdp_mul.o pdp_randmix.o \
	pdp_snap.o pdp_trigger.o pdp_bq.o pdp_noise.o pdp_gradient.o \
	pdp_route.o pdp_gain.o pdp_grey.o pdp_chrot.o pdp_scope.o \
	pdp_scale.o pdp_zoom.o

all_modules: $(OBJECTS) 

clean:
	rm -f *~
	rm -f *.o

