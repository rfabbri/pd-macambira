current: all_modules

include ../../Makefile.config

PDP_MOD = pdp_chrot.o  pdp_grey2mask.o  pdp_scale.o  pdp_scan.o \
	  pdp_scanxy.o  pdp_scope.o  pdp_slice_cut.o  pdp_slice_glue.o \
	pdp_cog.o pdp_histo.o pdp_array.o

# build special case image (and sound) processing modules
all_modules: $(PDP_MOD)

clean:
	rm -f *~
	rm -f *.o

