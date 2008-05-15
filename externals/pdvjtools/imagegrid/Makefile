PD_SRC=../../src

NAME=imagegrid
CSYM=imagegrid

current: pd_linux

# ----------------------- LINUX -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -fPIC -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch


LINUXINCLUDE =  -I$(PD_SRC) -I/usr/include/

.c.pd_linux:
	cc -fPIC -c -ansi -O -Wall -Wmissing-prototypes cua.c
	cc -fPIC -c -ansi -O -Wall -Wmissing-prototypes magickconverter.c
# 	./tk2c.bash < $*.tk > $*.tk2c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o cua.o magickconverter.o -lc -lm -lMagick `Wand-config --ldflags --libs`
	strip --strip-unneeded $*.pd_linux
	rm -f $*.o ../$*.pd_linux
	ln -s $*/$*.pd_linux ..

# ----------------------------------------------------------

install:
	cp *-help.pd ../../doc/5.reference

clean:
	rm -f *.o *.pd_* so_locations

