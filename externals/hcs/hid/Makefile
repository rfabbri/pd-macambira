CC=gcc

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
LDFLAGS = -bundle  -bundle_loader $(PDEXECUTABLE) -L/sw/lib
.SUFFIXES: .pd_darwin

all: input_arrays pd_darwin
pd_darwin: hid.pd_darwin

endif

# ----------------------- GENERAL -----------------------

PDEXECUTABLE = ../../../pd/bin/pd

CFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDE =  -I../ -I../../../pd/src -I/usr/local/include -I./HID\ Utilities\ Source

.c.pd_darwin:
	$(CC) $(CFLAGS) $(INCLUDE) -o $*.o -c $*.c
	$(CC) $(LDFLAGS) -o "$*.pd_darwin" "$*.o" -lc -lm 

.c.pd_linux:
	$(CC) $(CFLAGS) $(INCLUDE) -o $*.o -c $*.c
	ld $(LDFLAGS) -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

input_arrays:
	./make-arrays-from-input.h.pl > input_arrays.h


clean: ; rm -f *.pd_* *.o *~ input_arrays.h

