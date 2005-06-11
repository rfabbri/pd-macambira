
OS_NAME = $(shell uname -s)


# ----------------------- GNU/LINUX i386 -----------------------
ifeq ($(OS_NAME),Linux)
LDFLAGS = -export_dynamic  -shared
.SUFFIXES: .pd_linux

all: input_arrays pd_linux
pd_linux: hid.pd_linux

endif

# ----------------------- DARWIN -----------------------
ifeq ($(OS_NAME),Darwin)
FRAMEWORKS = Carbon IOKit ForceFeedback
LDFLAGS = -bundle  -bundle_loader $(PDEXECUTABLE) \
	       -L/sw/lib -L./HID\ Utilities\ Source/build \
	       -lHIDUtilities \
			 $(patsubst %,-framework %,$(FRAMEWORKS))
.SUFFIXES: .pd_darwin

all: input_arrays hid_utilities pd_darwin
pd_darwin: hid.pd_darwin

endif

# ----------------------- GENERAL ---------------------------------------------

PDEXECUTABLE = ../../../pd/bin/pd

# generic optimization
OPT_FLAGS = -O3 -ffast-math
# G4 optimization on Mac OS X
#OPT_FLAGS = -O3 -mcpu=7400 -maltivec -ffast-math -fPIC
# faster G4 7450 optimization  (gives errors) on GNU/Linux
#OPT_FLAGS = -O3 -mcpu=7450 -maltivec -ffast-math -fPIC
# G4 optimization on Mac OS X
#OPT_FLAGS = -O3 -mcpu=7400 -faltivec -ffast-math -fPIC
# faster G4 7450 optimization  (gives errors) on Mac OS X
#OPT_FLAGS = -ffast -mcpu=7450 -faltivec -ffast-math -fPIC

CFLAGS = $(OPT_FLAGS) -Wall -W -Wno-shadow -Wstrict-prototypes -Wno-unused

INCLUDE =  -I./ -I../../../pd/src -I./HID\ Utilities\ Source

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c *.c

.o.pd_darwin:
	$(CC) $(LDFLAGS) -o $*.pd_darwin *.o

.o.pd_linux:
	ld $(LDFLAGS) -o $*.pd_linux *.o -lc -lm
	strip --strip-unneeded $*.pd_linux

input_arrays:
	test -f input_arrays.h || ./make-arrays-from-input.h.pl

hid_utilities:
	test -f ./HID\ Utilities\ Source/build/libHIDUtilities.a || \
		( cd  ./HID\ Utilities\ Source && pbxbuild )

clean: ; rm -f *.pd_* *.o *~ input_arrays.? doc/ev*-list.pd

