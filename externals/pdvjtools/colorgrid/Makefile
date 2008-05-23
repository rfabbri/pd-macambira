NAME=colorgrid
CSYM=colorgrid

current: pd_linux


# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch #-Werror

LINUXINCLUDE =  -I../../src -I/usr/src/pd-0.40-2/src

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm -f $*.o ../$*.pd_linux
	ln -s $*/$*.pd_linux ..

# ----------------------------------------------------------

install:
	cp help-*.pd ../../doc/5.reference

clean:
	rm -f *.o *.pd_* so_locations
