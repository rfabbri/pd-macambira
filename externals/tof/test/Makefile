PARAM: param paramDump paramRoute paramFile

current:
	echo make pd_linux

clean: 
	rm -f *.pd_linux *.o

# ----------------------- LINUX i386 -----------------------

#paramL:  param.pd_linux

#patchArguments: patchArguments.pd_linux

.SUFFIXES: .pd_linux

PDPATH = /home/tom/src/pure-data/pd

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I$(PDPATH)/src

#.pd_linux: .c

.c:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld --export-dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o
