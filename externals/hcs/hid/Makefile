CC=gcc

all: input_arrays pd_darwin

pd_darwin: hid.pd_darwin macosxhid.pd_darwin

clean: ; rm -f *.pd_darwin *.o *~ input_arrays.h

# ----------------------- DARWIN i386 -----------------------

.SUFFIXES: .pd_darwin

PDEXECUTABLE = ../../../pd/bin/pd

CFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDE =  -I../ -I../../../pd/src -I/usr/local/include -I./HID\ Utilities\ Source

LDFLAGS = -bundle  -bundle_loader $(PDEXECUTABLE) -L/sw/lib

.c.pd_darwin:
	$(CC) $(CFLAGS) $(INCLUDE) -o $*.o -c $*.c
	$(CC) $(LDFLAGS) -o "$*.pd_darwin" "$*.o" -lc -lm 

input_arrays:
	./make-arrays-from-input.h.pl > input_arrays.h
