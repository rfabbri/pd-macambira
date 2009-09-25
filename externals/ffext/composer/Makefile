#!/usr/bin/make

DEBUG?=0
OS := $(shell uname -s)
TCL_VERSION := $(shell echo 'puts $$tcl_version' | tclsh)

ifeq ($(DEBUG),0)
	CFLAGS += -O2
else
	CFLAGS += -O0 -g -ggdb -DDEBUG
endif
ifeq ($(OS),Linux)
  PDSUF = .pd_linux
  PDBUNDLEFLAGS = -shared -rdynamic
  LDSOFLAGS = -lm
endif
ifeq ($(OS),Darwin)
  PDSUF = .pd_darwin
  INCLUDES = -I/Library/Frameworks/Tcl.framework/Headers
  PDBUNDLEFLAGS = -bundle -flat_namespace -undefined dynamic_lookup
  LDSOFLAGS = -lm
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  PDSUF = .dll
  PDBUNDLEFLAGS = -shared
  LDSOFLAGS = -lm
endif

LIBNAME = composer
INCLUDES =  -I../../pd/src -I/usr/include #-I/usr/include/tcl$(TCL_VERSION)
CFLAGS += -funroll-loops -fno-operator-names -fno-omit-frame-pointer -falign-functions=16 -Wall -fPIC
CFLAGS += -DPDSUF=\"$(PDSUF)\"
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)

all: $(LIBNAME)$(PDSUF)
	@echo '-----------------------------------------------------------------------------'
	@echo ' $(LIBNAME)$(PDSUF) ('`test $(DEBUG) -eq 1 && echo debug || echo release`' build) '\
		'[size: '`ls -gGh $(LIBNAME)$(PDSUF) | cut -d " " -f 3`']'

clean::
	rm -f $(LIBNAME)$(PDSUF) *.o *~

.SUFFIXES: .cpp .o

SRCS = Song.cpp Pattern.cpp Track.cpp PdClasses.cpp
OBJS = ${SRCS:.cpp=.o}

Song.o: Song.cpp Song.hpp
Pattern.o: Pattern.cpp Pattern.hpp
Track.o: Track.cpp Track.hpp
PdClass.o: PdClasses.cpp PdClasses.h

.cpp.o: Makefile
	$(CXX) $(CFLAGS) $(INCLUDES) -xc++ -c $<

$(LIBNAME)$(PDSUF): Makefile $(OBJS)
	$(LDSHARED) $(LDSOFLAGS) $(CFLAGS) -xnone $(OBJS) -o $(LIBNAME)$(PDSUF)

