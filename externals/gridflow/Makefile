#!/usr/bin/make
# $Id: Makefile 4238 2009-10-18 09:55:11Z mescalinum $

include config.make
# COMMON_DEPS = config.make Makefile base/source_filter.rb
RUBY = ruby

#--------#

SHELL = /bin/sh
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)
RM = rm -f
CFLAGS += -Wall -Wno-unused -Wunused-variable -g -fPIC -I.

# LDFLAGS += ../gem-cvs/Gem/Gem.pd_linux

LDSOFLAGS += -lm $(LIBS)
OBJS2 = base/grid.o base/flow_objects.o base/flow_objects2.o base/number.1.o base/number.2.o base/number.3.o base/number.4.o format/main.o
SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')
FILT = $(RUBY) -w base/source_filter.rb
ifeq ($(OS),darwin)
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -flat_namespace -undefined suppress
else
  ifeq ($(OS),nt)
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared
  else
    PDSUF = .pd_linux
    PDBUNDLEFLAGS = -shared -rdynamic
  endif
endif
PD_LIB = gridflow$(PDSUF)

ifeq ($(CPLUS_INCLUDE_PATH),)
  SNAFU =
else
  SNAFU = $(subst :, -I,:$(CPLUS_INCLUDE_PATH))
endif

all:: $(PD_LIB) deprecated

.SUFFIXES:

H = gridflow.h.fcs

%.h.fcs: %.h $(COMMON_DEPS)
	$(FILT) $< $@
%.c.fcs: %.c $(COMMON_DEPS) $(H)
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS) $(H)
	$(FILT) $< $@
%.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -c $< -o $@
%.1.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) $(SNAFU) -xobjective-c++ -c $< -o $@

%.s: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -S $< -o $@
%.e: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -E $< -o $@

.PRECIOUS: %.h.fcs %.c.fcs %.m.fcs

base/mmx.asm base/mmx_loader.c: base/mmx.rb
	$(RUBY) base/mmx.rb base/mmx.asm base/mmx_loader.c
base/mmx.o: base/mmx.asm
	nasm -f elf base/mmx.asm -o base/mmx.o

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

$(PD_LIB): gridflow.c.fcs $(OBJS2) $(OBJS) $(H) $(COMMON_DEPS)
	$(CXX) -DPDSUF=\"$(PDSUF)\" -Ibundled/pd $(LDSOFLAGS) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH) \
		gridflow.c.fcs -xnone $(OBJS2) $(OBJS) -o $@

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install::
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"

DEPRECATED = camera_control motion_detection color mouse fade scale_to \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb rotate in out

deprecated:: deprecated/@fade.pd deprecated/@!.pd doc/flow_classes/@complex_sq-help.pd

deprecated/@fade.pd: abstractions/\#fade.pd
	for z in $(DEPRECATED); do cp abstractions/\#$$z.pd deprecated/\@$$z.pd; done

deprecated/@!.pd: deprecated/0x40!.pd
	for z in complex_sq convolve fold inner \! scan; do cp deprecated/0x40$$z.pd deprecated/@$$z.pd; done

doc/flow_classes/@complex_sq-help.pd: doc/flow_classes/0x40complex_sq-help.pd
	for z in complex_sq join; do cp doc/flow_classes/0x40$$z-help.pd doc/flow_classes/@$$z-help.pd; done

clean::
	@-$(RM) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS2) $(OBJS) base/*.fcs format/*.fcs optional/*.fcs \
		$(patsubst %,deprecated/@%.pd,$(DEPRECATED))

distclean:: clean
	rm -f config.make config.log config.h gridflow.c.fcs gridflow.h.fcs
	rm -rf tmp

#--------#--------#--------#--------#--------#--------#--------#--------

help::
	@echo "do one of the following:";\
	echo  "make all            compiles gridflow";\
	echo  "make beep           beeps";\
	echo  "make unskew         removes timestamps in the future (if you have clock issues)"

#--------#--------#--------#--------#--------#--------#--------#--------

kloc::
	wc configure base/*.rb
