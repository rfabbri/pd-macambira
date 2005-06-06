
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

all: input_arrays pd_darwin
pd_darwin: hid.pd_darwin

endif

# ----------------------- GENERAL ---------------------------------------------

PDEXECUTABLE = ../../../pd/bin/pd

# generic optimization
OPT_FLAGS = -O3
# G4 7450 optimization  (gives errors)
#OPT_FLAGS = -fast -mcpu=7450 -maltivec

CFLAGS = $(OPT_FLAGS) -Wall -W -Wno-shadow -Wstrict-prototypes -Wno-unused

INCLUDE =  -I./ -I../../../pd/src -I./HID\ Utilities\ Source

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c *.c

.o.pd_darwin:
	$(CC) $(LDFLAGS) -o $*.pd_darwin *.o

.o.pd_linux:
	ld $(LDFLAGS) -o $*.pd_linux *.o -lc -lm
	strip --strip-unneeded $*.pd_linux
#	rm $*.o

input_arrays:
	./make-arrays-from-input.h.pl


clean: ; rm -f *.pd_* *.o *~ input_arrays.? doc/ev*-list.pd

