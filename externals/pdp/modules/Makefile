current: all_modules

include ../Makefile.config

PDP_MOD = pdp_add.o pdp_reg.o pdp_conv.o \
	pdp_mix.o pdp_affine.o pdp_del.o pdp_mul.o pdp_randmix.o \
	pdp_snap.o pdp_trigger.o pdp_bq.o pdp_noise.o pdp_gradient.o \
	pdp_route.o pdp_gain.o pdp_grey.o pdp_chrot.o pdp_scope.o \
	pdp_scale.o pdp_zoom.o pdp_scan.o pdp_cheby.o pdp_scanxy.o \
	pdp_grey2mask.o pdp_constant.o pdp_slice_cut.o pdp_slice_glue.o

# build portable and optional modules
all_modules: $(PDP_MOD) $(PDP_OPTMOD)

clean:
	rm -f *~
	rm -f *.o

