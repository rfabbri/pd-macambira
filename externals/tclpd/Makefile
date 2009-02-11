#!/usr/bin/make

TCL_VERSION := $(shell (ls -1d /usr/include/tcl8.? | sed -n 's|.*/tcl\(8\.[0-9][0-9]*\).*|\1|p') || echo "")
TCL_INCLUDES := -I$(shell find /usr/include -name tcl.h | grep -v private | sed 's|/tcl.h$$||')
INCLUDES =  -I../../pd/src -I/usr/include $(TCL_INCLUDES)
CFLAGS += $(INCLUDES) -xc++ -funroll-loops -fno-operator-names -fno-omit-frame-pointer -falign-functions=16 -O2 -Wall -fPIC
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
