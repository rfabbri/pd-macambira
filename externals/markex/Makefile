CC=gcc

current: pd_darwin

# ----------------------- DARWIN -----------------------

# replace this line with the one above, since this one has been hacked up
pd_darwin: alternate.pd_darwin gem_average.pd_darwin gem_change.pd_darwin gem_counter.pd_darwin invert.pd_darwin multiselect.pd_darwin oneshot.pd_darwin randomF.pd_darwin strcat.pd_darwin tripleLine.pd_darwin tripleRand.pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DMACOSX -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

DARWININCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src -I /usr/local/include

.c.pd_darwin:
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -o $*.o -c $*.c
	$(CC) -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o -lc -lm
	rm $*.o


# ----------------------- LINUX -----------------------

pd_linux: alternate.pd_linux gem_average.pd_linux gem_change.pd_linux gem_counter.pd_linux invert.pd_linux multiselect.pd_linux oneshot.pd_linux randomF.pd_linux strcat.pd_linux tripleLine.pd_linux tripleRand.pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src

.c.pd_linux:
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

# ----------------------- CLEAN -----------------------

clean: ; rm -f *.pd_* *.o *~

