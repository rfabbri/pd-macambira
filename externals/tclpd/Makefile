#!/usr/bin/make

DEBUG=1
TCL_VERSION := $(shell echo 'puts $$tcl_version' | tclsh)
INCLUDES =  -I../../pd/src -I/usr/include -I/usr/include/tcl$(TCL_VERSION)
CFLAGS += $(INCLUDES) -xc++ -funroll-loops -fno-operator-names -fno-omit-frame-pointer -falign-functions=16 -O2 -Wall -fPIC
ifeq ($(DEBUG),1)
	CFLAGS += -O0 -g -ggdb -DDEBUG
endif
LDSOFLAGS += -lm -ltcl$(TCL_VERSION)
CXX = g++
OS = linux
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)

all:: tcl

clean::
	rm -f tcl.pd_linux tcl_wrap.cxx *.o *~

.SUFFIXES:

ifeq ($(OS),darwin)
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -flat_namespace -undefined dynamic_lookup
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
	swig -v -c++ -tcl -o tcl_wrap.cxx $(INCLUDES) tcl.i
