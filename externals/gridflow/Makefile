#!/usr/bin/make
# $Id: Makefile 4724 2009-11-03 20:57:34Z matju $

include config.make
COMMON_DEPS = config.make Makefile src/source_filter.rb
COMMON_DEPS2 = $(COMMON_DEPS) src/gridflow.hxx.fcs
RUBY = ruby

#--------#

SHELL = /bin/sh
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)
RM = rm -f
CFLAGS += -Wall -Wno-unused -Wunused-variable -g -fPIC -I.

# LDFLAGS += ../gem-cvs/Gem/Gem.pd_linux

LDSOFLAGS += -lm $(LIBS)
OBJS2 = src/gridflow.o src/grid.o src/classes1.o src/classes2.o src/number.1.o src/number.2.o src/number.3.o src/number.4.o src/formats.o
SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')
FILT = $(RUBY) -w src/source_filter.rb
ifeq ($(OS),darwin)
  CFLAGS += -mmacosx-version-min=10.4
  LDSOFLAGS += -headerpad_max_install_names
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

all:: $(PD_LIB) aliases

.SUFFIXES:

CFLAGS += -DPDSUF=\"$(PDSUF)\"

%.hxx.fcs: %.hxx $(COMMON_DEPS)
	$(FILT) $< $@
%.cxx.fcs: %.cxx $(COMMON_DEPS2)
	$(FILT) $< $@
%.c.fcs: %.c $(COMMON_DEPS2)
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS2)
	$(FILT) $< $@
%.o: %.c.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -c $< -o $@
%.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -c $< -o $@
%.1.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) $(SNAFU) -xobjective-c++ -c $< -o $@

%.s: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) $(CFLAGS) -S $< -o $@
%.e: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) $(CFLAGS) -E $< -o $@

.PRECIOUS: %.hxx.fcs %.cxx.fcs %.h.fcs %.c.fcs %.m.fcs

src/mmx.asm src/mmx_loader.cxx: src/mmx.rb
	$(RUBY) src/mmx.rb src/mmx.asm src/mmx_loader.cxx
src/mmx.o: src/mmx.asm
	nasm -f elf src/mmx.asm -o src/mmx.o

$(PD_LIB): $(OBJS2) $(OBJS) $(H) $(COMMON_DEPS)
	$(CXX) -DPDSUF=\"$(PDSUF)\" $(LDSOFLAGS) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH) -xnone $(OBJS2) $(OBJS) -o $@

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install::
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux if you have one (from years ago)"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"

DEPRECATED = motion_detection color mouse fade scale_to \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb rotate in out

aliases:: deprecated/@fade.pd deprecated/@!.pd doc/flow_classes/@complex_sq-help.pd \
  doc/flow_classes/inv+-help.pd  abstractions/inv+.pd \
  doc/flow_classes/inv\*-help.pd abstractions/inv\*.pd

deprecated/@fade.pd: abstractions/\#fade.pd
	for z in $(DEPRECATED); do cp abstractions/\#$$z.pd deprecated/\@$$z.pd; done

deprecated/@!.pd: deprecated/0x40!.pd
	for z in complex_sq convolve fold inner \! scan; do cp deprecated/0x40$$z.pd deprecated/@$$z.pd; done

doc/flow_classes/@complex_sq-help.pd: doc/flow_classes/0x40complex_sq-help.pd
	cp doc/flow_classes/0x40complex_sq-help.pd doc/flow_classes/@complex_sq-help.pd

doc/flow_classes/inv+-help.pd: doc/flow_classes/inv0x2b-help.pd
	cp doc/flow_classes/inv0x2b-help.pd doc/flow_classes/inv+-help.pd

doc/flow_classes/inv\*-help.pd: doc/flow_classes/inv0x2a-help.pd
	cp doc/flow_classes/inv0x2a-help.pd doc/flow_classes/inv\*-help.pd

abstractions/inv+.pd: abstractions/inv0x2b.pd
	cp abstractions/inv0x2b.pd abstractions/inv+.pd

abstractions/inv\*.pd: abstractions/inv0x2a.pd
	cp abstractions/inv0x2a.pd abstractions/inv\*.pd

clean::
	@-$(RM) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS2) $(OBJS)src/*.fcs format/*.fcs $(patsubst %,deprecated/@%.pd,$(DEPRECATED))

distclean:: clean
	rm -f config.make config.log config.h
	rm -rf tmp
