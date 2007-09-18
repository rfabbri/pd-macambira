#!/usr/bin/make

CPU=athlon-xp
CFLAGS += -I/usr/include -I. -xc++ -funroll-loops -fno-operator-names -fno-omit-frame-pointer -falign-functions=16 -mtune=$(CPU) -march=$(CPU) -Wall -Wno-unused -Wunused-variable -Wno-strict-aliasing -g -fPIC -I.
LDSOFLAGS += -lm -L/usr/lib -ltcl8.5 -L/usr/X11R6/lib
CXX = g++
OS = linux
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)

all:: tcl

clean::
	rm -f tcl.pd_linux tcl_wrap.cxx *.o *~

.SUFFIXES:

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

tcl:: tcl.pd_linux

tcl.pd_linux: tcl_wrap.cxx tcl_extras.cxx tcl_loader.cxx tcl_extras.h Makefile
	$(LDSHARED) $(CFLAGS) -DPDSUF=\"$(PDSUF)\" -o tcl$(PDSUF) \
		tcl_wrap.cxx tcl_extras.cxx tcl_loader.cxx $(LDSOFLAGS)

tcl_wrap.cxx: tcl.i tcl_extras.h
	swig -v -c++ -tcl -o tcl_wrap.cxx -I/usr/include -I/usr/local/include tcl.i

